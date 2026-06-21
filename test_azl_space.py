# test_azl_space.py - AZL substrate CI
# matches paidingattentionproductionsllc/Lattice Tier 1-6

import azl_space as azl
import pytest

def test_lookup_law():
    r = azl.azl_lookup(847293847)
    assert r["address"] == "AZL-0847293847"
    assert abs(r["value"] - 0.847293847) < 1e-12
    assert r["law"] == "N×0=N"
    assert r["proof"] == "1×1=2"

def test_lerp_midpoint():
    a = azl.azl_value(100)
    b = azl.azl_value(200)
    m = azl.lerp_pos(a, b, 0.5)
    assert abs(m - azl.azl_value(150)) < 1e-12

def test_greedy_moves_toward_target():
    cur = 0.1
    neighbors = [0.15, 0.5, 0.05]
    nxt = azl.greedy_next_hop(cur, neighbors, 0.6)
    assert nxt == 0.5

def test_route_runs():
    r = azl.simulate_route(1000, 900000000, num_nodes=50)
    assert "hops" in r
    assert len(r["path_addresses"]) >= 1

# --- new coverage tests ---

def test_dist_and_pos_snap():
    assert azl.dist(0.5, 0.2) == pytest.approx(0.3)
    # snap a substrate position back to lattice
    pos = azl.azl_value(42)
    assert azl.pos_to_nearest_azl(pos) == 42
    assert azl.pos_to_nearest_azl(0.0) == azl.AZL_MIN

def test_azl_address_alias():
    # overlay compatibility
    assert azl.azl_address(7) == "AZL-0000000007"
    assert azl.azl_address(7) == azl.azl_id(7)

def test_lookup_bounds():
    with pytest.raises(ValueError):
        azl.azl_lookup(0)
    with pytest.raises(ValueError):
        azl.azl_lookup(azl.AZL_MAX_TIER_6 + 1)

def test_greedy_empty_neighbors():
    # should stay put, not crash
    assert azl.greedy_next_hop(0.5, [], 0.9) == 0.5

def test_route_is_deterministic():
    r1 = azl.simulate_route(100, 900, num_nodes=50, seed=42)
    r2 = azl.simulate_route(100, 900, num_nodes=50, seed=42)
    assert r1["path_addresses"] == r2["path_addresses"]

if __name__ == "__main__":
    test_lookup_law()
    test_lerp_midpoint()
    test_greedy_moves_toward_target()
    test_route_runs()
    test_dist_and_pos_snap()
    test_azl_address_alias()
    test_lookup_bounds()
    test_greedy_empty_neighbors()
    test_route_is_deterministic()
    print("AZL space: 9/9 PASS - Green")
