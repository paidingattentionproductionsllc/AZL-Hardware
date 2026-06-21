#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "firmware/azl_space.h"
#include "firmware/azl_mesh.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static bool test_##name(void); \
    static bool run_##name(void) { \
        tests_run++; \
        printf("[%2d] %-32s... ", tests_run, #name); \
        bool ok = test_##name(); \
        if (ok) { tests_passed++; printf("PASS\n"); } else { printf("FAIL\n"); } \
        return ok; \
    } \
    static bool test_##name(void)

#define ASSERT(cond) do { if (!(cond)) { printf("\n ASSERT failed: %s at %s:%d\n", #cond, __FILE__, __LINE__); return false; } } while(0)
#define ASSERT_EQ_U32(a,b) ASSERT((a) == (b))
#define ASSERT_NEAR_F(a,b,eps) ASSERT(fabsf((a)-(b)) < (eps))

/* --- space tests --- */

TEST(lookup_law) {
    /* N×0=N, 1×1=2 – manifest constants present */
    ASSERT(strcmp(AZL_LAW, "N×0=N") == 0);
    ASSERT(strcmp(AZL_PROOF, "1×1=2") == 0);
    return true;
}

TEST(lookup_bounds) {
    ASSERT(azl_address_valid(AZL_ADDR_MIN));
    ASSERT(azl_address_valid(AZL_ADDR_MAX));
    ASSERT(!azl_address_valid(0));
    ASSERT(!azl_address_valid(AZL_ADDR_MAX + 1));
    float v_first = azl_value_f(AZL_ADDR_MIN);
    float v_last = azl_value_f(AZL_ADDR_MAX);
    ASSERT_NEAR_F(v_first, 1e-9f, 1e-12f);
    ASSERT_NEAR_F(v_last, 1.0f, 1e-6f);
    return true;
}

TEST(lerp_midpoint) {
    float v_mid = azl_value_f(AZL_ADDR_MID);
    ASSERT_NEAR_F(v_mid, 0.5f, 1e-6f);
    uint32_t n = azl_pos_to_nearest_f(0.5f);
    ASSERT_EQ_U32(n, AZL_ADDR_MID);
    return true;
}

TEST(dist_and_pos_snap) {
    float a = azl_value_f(100000000);
    float b = azl_value_f(300000000);
    float d = azl_dist_f(a, b);
    ASSERT_NEAR_F(d, 0.2f, 1e-6f);
    uint32_t n = azl_pos_to_nearest_f(0.123456789f);
    ASSERT(n >= AZL_ADDR_MIN && n <= AZL_ADDR_MAX);
    /* snap round-trip */
    float v = azl_value_f(n);
    uint32_t n2 = azl_pos_to_nearest_f(v);
    ASSERT_EQ_U32(n, n2);
    return true;
}

TEST(azl_address_alias) {
    char buf[16];
    azl_format(buf, sizeof(buf), 1);
    ASSERT(strcmp(buf, "AZL-0000000001") == 0);
    azl_format(buf, sizeof(buf), 500000000);
    ASSERT(strcmp(buf, "AZL-0500000000") == 0);
    azl_format(buf, sizeof(buf), 1000000000);
    ASSERT(strcmp(buf, "AZL-1000000000") == 0);
    return true;
}

TEST(greedy_moves_toward_target) {
    uint32_t neighbors[] = {100000000, 400000000, 800000000};
    uint32_t next = azl_greedy_next_hop(100000000, 900000000, neighbors, 3);
    /* closest to 0.9 is 800M */
    ASSERT_EQ_U32(next, 800000000);
    return true;
}

TEST(greedy_empty_neighbors) {
    uint32_t next = azl_greedy_next_hop(100, 200, NULL, 0);
    ASSERT_EQ_U32(next, 0);
    return true;
}

TEST(route_runs) {
    azl_mesh_init();
    azl_mesh_add_link(100000000, 300000000);
    azl_mesh_add_link(300000000, 600000000);
    azl_mesh_route_result_t r = azl_mesh_route(100000000, 600000000);
    ASSERT(r.hops > 0);
    return true;
}

TEST(route_is_deterministic) {
    azl_mesh_init();
    azl_mesh_add_link(10, 20);
    azl_mesh_add_link(20, 30);
    azl_mesh_route_result_t r1 = azl_mesh_route(10, 30);
    azl_mesh_route_result_t r2 = azl_mesh_route(10, 30);
    ASSERT(r1.delivered == r2.delivered);
    ASSERT_EQ_U32(r1.hops, r2.hops);
    ASSERT_EQ_U32(r1.final_addr, r2.final_addr);
    return true;
}

/* --- overlay tests --- */

TEST(direct_neighbor) {
    azl_mesh_init();
    azl_mesh_add_link(100, 200);
    azl_mesh_route_result_t r = azl_mesh_route(100, 200);
    ASSERT(r.delivered);
    ASSERT_EQ_U32(r.hops, 1);
    ASSERT_EQ_U32(r.final_addr, 200);
    return true;
}

TEST(ring_route) {
    azl_mesh_init();
    /* 100 -> 200 -> 300 -> 400 ring */
    azl_mesh_add_link(100, 200);
    azl_mesh_add_link(200, 300);
    azl_mesh_add_link(300, 400);
    azl_mesh_route_result_t r = azl_mesh_route(100, 400);
    ASSERT(r.delivered);
    ASSERT(r.hops >= 1 && r.hops <= 10);
    ASSERT_EQ_U32(r.final_addr, 400);
    return true;
}

TEST(no_loop) {
    azl_mesh_init();
    /* triangle: 100-200-300-100 */
    azl_mesh_add_link(100, 200);
    azl_mesh_add_link(200, 300);
    azl_mesh_add_link(300, 100);
    azl_mesh_route_result_t r = azl_mesh_route(100, 300);
    ASSERT(r.delivered);
    /* path must have no duplicates */
    for (uint8_t i = 0; i < r.path_len; i++) {
        for (uint8_t j = i + 1; j < r.path_len; j++) {
            ASSERT(r.path[i]!= r.path[j]);
        }
    }
    return true;
}

TEST(unreachable) {
    azl_mesh_init();
    azl_mesh_add_node(100);
    azl_mesh_add_node(900000000);
    /* no link between them */
    azl_mesh_route_result_t r = azl_mesh_route(100, 900000000);
    ASSERT(!r.delivered);
    return true;
}

TEST(self_route) {
    azl_mesh_init();
    azl_mesh_add_node(12345);
    azl_mesh_route_result_t r = azl_mesh_route(12345, 12345);
    ASSERT(r.delivered);
    ASSERT_EQ_U32(r.hops, 0);
    ASSERT_EQ_U32(r.final_addr, 12345);
    return true;
}

TEST(duplicate_link_is_idempotent) {
    azl_mesh_init();
    bool ok1 = azl_mesh_add_link(100, 200);
    bool ok2 = azl_mesh_add_link(100, 200);
    bool ok3 = azl_mesh_add_link(200, 100);
    ASSERT(ok1 && ok2 && ok3);
    azl_mesh_route_result_t r = azl_mesh_route(100, 200);
    ASSERT(r.delivered);
    return true;
}

TEST(payload_echo) {
    azl_mesh_init();
    azl_mesh_add_link(10, 20);
    uint8_t payload[] = {0xAA, 0xBB, 0xCC};
    azl_mesh_route_result_t r;
    bool sent = azl_mesh_send(10, 20, payload, sizeof(payload), &r);
    ASSERT(sent);
    ASSERT(r.delivered);
    return true;
}

TEST(missing_node_unreachable) {
    azl_mesh_init();
    azl_mesh_add_node(100);
    /* dst 999 never added */
    azl_mesh_route_result_t r = azl_mesh_route(100, 999);
    ASSERT(!r.delivered);
    return true;
}

/* --- runner --- */

int main(void) {
    printf("AZL Firmware Test Harness\n");
    printf("Manifest %s, Tier %d-%d, %s / %s\n\n",
        AZL_VERSION, AZL_TIER_MIN, AZL_TIER_MAX, AZL_LAW, AZL_PROOF);

    run_lookup_law();
    run_lookup_bounds();
    run_lerp_midpoint();
    run_dist_and_pos_snap();
    run_azl_address_alias();
    run_greedy_moves_toward_target();
    run_greedy_empty_neighbors();
    run_route_runs();
    run_route_is_deterministic();

    run_direct_neighbor();
    run_ring_route();
    run_no_loop();
    run_unreachable();
    run_self_route();
    run_duplicate_link_is_idempotent();
    run_payload_echo();
    run_missing_node_unreachable();

    printf("\n%d / %d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run)? 0 : 1;
}
