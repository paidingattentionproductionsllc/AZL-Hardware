#ifndef AZL_MESH_H
#define AZL_MESH_H
/*
 * AZL Mesh – Firmware overlay
 * Matches: azl_overlay.py in AZL-Hardware
 * Test suite: test_azl_overlay.py – 8 tests green in CI #20/#21
 * Depends on: azl_space.h (manifest v1.0, CC-BY-4.0)
 */

#include "azl_space.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AZL_MESH_MAX_NODES
#define AZL_MESH_MAX_NODES 64
#endif

#ifndef AZL_MESH_MAX_NEIGHBORS
#define AZL_MESH_MAX_NEIGHBORS 16
#endif

#ifndef AZL_MESH_DEFAULT_TTL
#define AZL_MESH_DEFAULT_TTL 32
#endif

#define AZL_MESH_MAX_PAYLOAD 64

typedef struct {
    uint32_t addr;
    uint32_t neighbors[AZL_MESH_MAX_NEIGHBORS];
    uint8_t neighbor_count;
    uint8_t in_use;
} azl_mesh_node_t;

typedef struct {
    uint32_t src;
    uint32_t dst;
    uint8_t hops;
    uint8_t ttl;
    uint8_t payload_len;
    uint8_t payload[AZL_MESH_MAX_PAYLOAD];
} azl_mesh_packet_t;

typedef struct {
    bool delivered;
    uint32_t final_addr;
    uint8_t hops;
    uint32_t path[AZL_MESH_DEFAULT_TTL];
    uint8_t path_len;
} azl_mesh_route_result_t;

/* Mesh management */
void azl_mesh_init(void);
void azl_mesh_clear(void);
bool azl_mesh_add_node(uint32_t addr);
bool azl_mesh_add_link(uint32_t a, uint32_t b); /* idempotent, bidirectional */

/* Routing – greedy AZL, loop-safe */
azl_mesh_route_result_t azl_mesh_route(uint32_t src, uint32_t dst);
azl_mesh_route_result_t azl_mesh_route_with_ttl(uint32_t src, uint32_t dst, uint8_t ttl);

/* Packet helpers – payload echo test */
bool azl_mesh_send(
    uint32_t src,
    uint32_t dst,
    const uint8_t *payload,
    uint8_t payload_len,
    azl_mesh_route_result_t *result_out
);

#ifdef __cplusplus
}
#endif

#endif /* AZL_MESH_H */
