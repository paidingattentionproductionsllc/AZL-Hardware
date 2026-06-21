#include "azl_mesh.h"
#include <string.h>

static azl_mesh_node_t mesh_nodes[AZL_MESH_MAX_NODES];
static uint8_t mesh_node_count = 0;

static azl_mesh_node_t* find_node(uint32_t addr) {
    for (uint8_t i = 0; i < mesh_node_count; i++) {
        if (mesh_nodes[i].in_use && mesh_nodes[i].addr == addr) {
            return &mesh_nodes[i];
        }
    }
    return NULL;
}

static azl_mesh_node_t* get_or_create_node(uint32_t addr) {
    azl_mesh_node_t* n = find_node(addr);
    if (n) return n;
    if (mesh_node_count >= AZL_MESH_MAX_NODES) return NULL;
    n = &mesh_nodes[mesh_node_count++];
    memset(n, 0, sizeof(*n));
    n->addr = addr;
    n->in_use = 1;
    return n;
}

void azl_mesh_init(void) {
    memset(mesh_nodes, 0, sizeof(mesh_nodes));
    mesh_node_count = 0;
}

void azl_mesh_clear(void) {
    azl_mesh_init();
}

bool azl_mesh_add_node(uint32_t addr) {
    if (!azl_address_valid(addr)) return false;
    return get_or_create_node(addr)!= NULL;
}

static bool add_neighbor_one_way(azl_mesh_node_t* n, uint32_t nb_addr) {
    for (uint8_t i = 0; i < n->neighbor_count; i++) {
        if (n->neighbors[i] == nb_addr) return true; /* idempotent */
    }
    if (n->neighbor_count >= AZL_MESH_MAX_NEIGHBORS) return false;
    n->neighbors[n->neighbor_count++] = nb_addr;
    return true;
}

bool azl_mesh_add_link(uint32_t a, uint32_t b) {
    if (a == b) return true; /* self-link is a no-op, matches Python */
    if (!azl_address_valid(a) ||!azl_address_valid(b)) return false;
    azl_mesh_node_t* na = get_or_create_node(a);
    azl_mesh_node_t* nb = get_or_create_node(b);
    if (!na ||!nb) return false;
    bool ok_a = add_neighbor_one_way(na, b);
    bool ok_b = add_neighbor_one_way(nb, a);
    return ok_a && ok_b;
}

azl_mesh_route_result_t azl_mesh_route_with_ttl(uint32_t src, uint32_t dst, uint8_t ttl) {
    azl_mesh_route_result_t r;
    memset(&r, 0, sizeof(r));
    r.final_addr = src;

    if (src == dst) {
        r.delivered = true;
        r.hops = 0;
        r.path[0] = src;
        r.path_len = 1;
        return r;
    }

    uint32_t current = src;
    uint8_t hops = 0;
    r.path[r.path_len++] = current;

    /* simple visited set for loop detection */
    uint32_t visited[AZL_MESH_DEFAULT_TTL];
    uint8_t visited_count = 0;
    visited[visited_count++] = current;

    while (hops < ttl) {
        azl_mesh_node_t* node = find_node(current);
        if (!node || node->neighbor_count == 0) break;

        uint32_t next = azl_greedy_next_hop(
            current, dst,
            node->neighbors, node->neighbor_count
        );
        if (next == 0 || next == current) break;

        /* loop check */
        bool seen = false;
        for (uint8_t i = 0; i < visited_count; i++) {
            if (visited[i] == next) { seen = true; break; }
        }
        if (seen) break;

        current = next;
        hops++;
        if (r.path_len < AZL_MESH_DEFAULT_TTL) {
            r.path[r.path_len++] = current;
        }
        visited[visited_count++] = current;

        if (current == dst) {
            r.delivered = true;
            break;
        }
        if (visited_count >= AZL_MESH_DEFAULT_TTL) break;
    }

    r.final_addr = current;
    r.hops = hops;
    return r;
}

azl_mesh_route_result_t azl_mesh_route(uint32_t src, uint32_t dst) {
    return azl_mesh_route_with_ttl(src, dst, AZL_MESH_DEFAULT_TTL);
}

bool azl_mesh_send(
    uint32_t src,
    uint32_t dst,
    const uint8_t *payload,
    uint8_t payload_len,
    azl_mesh_route_result_t *result_out
) {
    if (payload_len > AZL_MESH_MAX_PAYLOAD) return false;
    azl_mesh_route_result_t r = azl_mesh_route(src, dst);
    if (result_out) *result_out = r;
    /* payload echo test: if delivered, payload is considered echoed */
    return r.delivered;
}
