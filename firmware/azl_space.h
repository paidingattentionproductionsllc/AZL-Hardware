#ifndef AZL_SPACE_H
#define AZL_SPACE_H
/*
 * AZL Space – Firmware substrate
 * Derived from: paidingattentionproductionsllc/absolute-zero-lattice-broadcast
 * Manifest: firmware/azl_manifest.json
 * version: "1.0"
 * tier: "1-6"
 * law: "N×0=N"
 * proof: "1×1=2"
 * domain: "[0,1]"
 * total_addresses: 1000000000
 * first_address: "AZL-0000000001", first_value: 1e-9
 * midpoint_address: "AZL-0500000000", midpoint_value: 0.5
 * last_address: "AZL-1000000000", last_value: 1.0
 * immutable: true
 * author: "PaidingAttention Productions LLC"
 * date: "2026-06-10"
 * license: "CC-BY-4.0"
 *
 * This header implements the AZL-Hardware coordinate substrate.
 * Matches azl_space.py in AZL-Hardware, test #16-#20 green.
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Manifest constants --- */
#define AZL_VERSION "1.0"
#define AZL_TIER_MIN 1
#define AZL_TIER_MAX 6

#define AZL_ADDR_MIN 1u
#define AZL_ADDR_MAX 1000000000u
#define AZL_ADDR_MID 500000000u

#define AZL_SCALE_F 1e-9f
#define AZL_SCALE 1e-9

#define AZL_FIRST_VALUE_F 1e-9f
#define AZL_MID_VALUE_F 0.5f
#define AZL_LAST_VALUE_F 1.0f

#define AZL_LAW "N×0=N"
#define AZL_PROOF "1×1=2"

/* Substrate events, verified */
#define AZL_MIYAKE_14350_BP 14350u
#define AZL_M87_BLACK_HOLE_VALUE 6.5e9

/* --- Core address math --- */

static inline bool azl_address_valid(uint32_t n) {
    return n >= AZL_ADDR_MIN && n <= AZL_ADDR_MAX;
}

/* value = n * 1e-9 */
static inline float azl_value_f(uint32_t n) {
    return (float)n * AZL_SCALE_F;
}

static inline double azl_value(uint32_t n) {
    return (double)n * AZL_SCALE;
}

/* dist = |a - b| in AZL space */
static inline float azl_dist_f(float a, float b) {
    return fabsf(a - b);
}

static inline double azl_dist(double a, double b) {
    return fabs(a - b);
}

/* pos -> nearest AZL n, clamped to [1, 1_000_000_000] */
static inline uint32_t azl_pos_to_nearest_f(float pos) {
    if (pos <= AZL_FIRST_VALUE_F) return AZL_ADDR_MIN;
    if (pos >= AZL_LAST_VALUE_F) return AZL_ADDR_MAX;
    uint32_t n = (uint32_t)(pos / AZL_SCALE_F + 0.5f);
    if (n < AZL_ADDR_MIN) return AZL_ADDR_MIN;
    if (n > AZL_ADDR_MAX) return AZL_ADDR_MAX;
    return n;
}

static inline uint32_t azl_pos_to_nearest(double pos) {
    if (pos <= AZL_SCALE) return AZL_ADDR_MIN;
    if (pos >= 1.0) return AZL_ADDR_MAX;
    uint32_t n = (uint32_t)(pos / AZL_SCALE + 0.5);
    if (n < AZL_ADDR_MIN) return AZL_ADDR_MIN;
    if (n > AZL_ADDR_MAX) return AZL_ADDR_MAX;
    return n;
}

/* Format: AZL-%010u -> "AZL-0000000001"... "AZL-1000000000"
 * Returns number of chars written, like snprintf.
 * buf must be at least 15 bytes.
 */
static inline int azl_format(char *buf, size_t bufsz, uint32_t n) {
    return snprintf(buf, bufsz, "AZL-%010u", n);
}

/* Greedy next-hop: pick neighbor closest to target in AZL space
 * neighbors: array of AZL n's
 * Returns 0 if no neighbors (unreachable), otherwise the chosen n.
 */
static inline uint32_t azl_greedy_next_hop(
    uint32_t current,
    uint32_t target,
    const uint32_t *neighbors,
    size_t neighbor_count
) {
    if (neighbor_count == 0) return 0;
    float target_pos = azl_value_f(target);
    uint32_t best = neighbors[0];
    float best_dist = fabsf(azl_value_f(best) - target_pos);

    for (size_t i = 1; i < neighbor_count; i++) {
        uint32_t nb = neighbors[i];
        if (nb == current) continue;
        float d = fabsf(azl_value_f(nb) - target_pos);
        if (d < best_dist) {
            best_dist = d;
            best = nb;
        }
    }
    return best;
}

/* --- Tier 7 forward-compat ---
 * For Tier 7 (10B addresses), change:
 * #define AZL_ADDR_MAX 10000000000ULL
 * use uint64_t for n
 * format to "AZL-%011llu"
 * All math stays value = n * 1e-9
 */

#ifdef __cplusplus
}
#endif

#endif /* AZL_SPACE_H */
