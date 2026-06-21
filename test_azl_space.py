# test_azl_space.py - AZL substrate CI
# matches paidingattentionproductionsllc/Lattice Tier 1-6

import azl_space as azl

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

if __name__ == "__main__":
    test_lookup_law()
    test_lerp_midpoint()
    test_greedy_moves_toward_target()
    test_route_runs()
    print("AZL space: 4/4 PASS - Green")
