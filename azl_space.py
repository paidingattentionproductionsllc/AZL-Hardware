"""
azl_space.py - AZL coordinate substrate
Matches paidingattentionproductionsllc/Lattice manifest

Addresses: AZL-0000000001 … AZL-1000000000
Value: n * 1e-9
Law field: "N×0=N"
Proof field: "1×1=2"

Tier 7: expand AZL_MAX to 10_000_000_000 for 10B addresses
"""

import math
import json
import random
from typing import List

# --- Lattice constants, per azl_manifest.json ---
AZL_MIN = 1
AZL_MAX_TIER_6 = 1_000_000_000
AZL_MAX_TIER_7 = 10_000_000_000

AZL_SCALE = 1e-9
AZL_LAW = "N×0=N"
AZL_PROOF = "1×1=2"

def azl_value(n: int) -> float:
    """Coordinate position in [0,1]"""
    return n * AZL_SCALE

def azl_id(n: int) -> str:
    return f"AZL-{n:010d}"

def azl_lookup(n: int, tier_max: int = AZL_MAX_TIER_6) -> dict:
    if not (AZL_MIN <= n <= tier_max):
        raise ValueError(f"n out of range 1..{tier_max}")
    return {
        "address": azl_id(n),
        "n": n,
        "value": azl_value(n),
        "law": AZL_LAW,
        "proof": AZL_PROOF
    }

# --- Substrate space: mapping *around* the addresses ---

def lerp_pos(a_pos: float, b_pos: float, t: float) -> float:
    """Interpolate in the space between two AZL positions. t in [0,1]"""
    return a_pos + (b_pos - a_pos) * t

def pos_to_nearest_azl(pos: float) -> int:
    """Snap a substrate position back to the nearest lattice address"""
    n = int(round(pos / AZL_SCALE))
    return max(AZL_MIN, min(n, AZL_MAX_TIER_6))

def dist(a: float, b: float) -> float:
    return abs(a - b)

def greedy_next_hop(current_pos: float, neighbors: List[float], target_pos: float) -> float:
    """Pick the neighbor closest to target. Classic virtual-coordinate greedy forward."""
    if not neighbors:
        return current_pos
    return min(neighbors, key=lambda p: dist(p, target_pos))

def simulate_route(start_n: int, target_n: int, num_nodes: int = 200, degree: int = 6, seed: int = 42) -> dict:
    """Small substrate routing sim. Nodes get random AZL positions."""
    random.seed(seed)
    start_pos = azl_value(start_n)
    target_pos = azl_value(target_n)

    # random lattice nodes
    nodes = sorted(set(random.randint(AZL_MIN, AZL_MAX_TIER_6) for _ in range(num_nodes)))
    node_pos = [azl_value(n) for n in nodes]

    # build a simple k-nearest neighbor graph in AZL space
    def k_nearest(i):
        dists = [(abs(node_pos[i] - node_pos[j]), j) for j in range(len(nodes)) if j!= i]
        dists.sort()
        return [node_pos[j] for _, j in dists[:degree]]

    # start at the lattice node nearest to start_pos
    current_idx = min(range(len(nodes)), key=lambda i: abs(node_pos[i] - start_pos))
    path = [node_pos[current_idx]]

    visited = set()
    for _ in range(50): # hop limit
        if dist(path[-1], target_pos) < 1e-7:
            break
        current_idx = node_pos.index(path[-1])
        if current_idx in visited:
            break
        visited.add(current_idx)
        neighbors = k_nearest(current_idx)
        next_pos = greedy_next_hop(path[-1], neighbors, target_pos)
        if next_pos == path[-1]:
            break
        path.append(next_pos)

    return {
        "start": azl_lookup(start_n),
        "target": azl_lookup(target_n),
        "hops": len(path) - 1,
        "path_addresses": [azl_id(pos_to_nearest_azl(p)) for p in path],
        "path_values": path
    }

if __name__ == "__main__":
    import sys
    if len(sys.argv) >= 3:
        s = int(sys.argv[1])
        t = int(sys.argv[2])
    else:
        s, t = 84729384, 847293847 # example
    result = simulate_route(s, t)
    print(json.dumps(result, indent=2))
