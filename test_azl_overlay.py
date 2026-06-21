# test_azl_overlay.py - AZL overlay CI
import azl_overlay as overlay

def test_direct_neighbor():
    m = overlay.AZLMesh()
    m.link(100, 200)
    r = m.route(100, 200, payload=b"hi")
    assert r["delivered"] is True
    assert r["hops"] == 1

def test_ring_route():
    ns = [1000, 150000, 400000000, 700000000, 900000000]
    m = overlay.build_ring(ns)
    r = m.route(ns[0], ns[-1])
    assert r["delivered"] is True
    assert r["hops"] <= len(ns)
    assert r["path"][0] == ns[0]
    assert r["path"][-1] == ns[-1]

def test_no_loop():
    # linear chain, route backwards - should not loop
    m = overlay.AZLMesh()
    for a, b in [(10,20), (20,30), (30,40)]:
        m.link(a, b)
    r = m.route(40, 10)
    assert r["delivered"] is True
    assert len(r["path"]) == len(set(r["path"]))  # no repeats

def test_unreachable():
    m = overlay.AZLMesh()
    m.add_node(1)
    m.add_node(999999999)
    # no link between them
    r = m.route(1, 999999999)
    assert r["delivered"] is False

if __name__ == "__main__":
    test_direct_neighbor()
    test_ring_route()
    test_no_loop()
    test_unreachable()
    print("AZL overlay: 4/4 PASS - Green")
