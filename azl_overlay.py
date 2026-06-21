#!/usr/bin/env python3
"""
azl_overlay.py - AZL coordinate overlay network
for paidingattentionproductionsllc / AZL-Hardware

Nodes are addressed by AZL n / value = n * 1e-9
Routing is greedy through the AZL substrate - forward to the neighbor
closest to the target coordinate.
Pure Python, no sockets - safe for CI. Swap the transport later for real UDP.
"""

from typing import Dict, List, Optional
import azl_space as azl

class AZLNode:
    def __init__(self, n: int):
        self.n = n
        self.value = azl.azl_value(n)
        self.address = azl.azl_address(n)
        self.neighbors: Dict[int, "AZLNode"] = {}
        self.inbox: List[dict] = []

    def add_peer(self, peer: "AZLNode"):
        if peer.n != self.n:
            self.neighbors[peer.n] = peer

    def route_next(self, target_value: float, visited: set) -> Optional["AZLNode"]:
        candidates = [p for p in self.neighbors.values() if p.n not in visited]
        if not candidates:
            return None
        neighbor_values = [p.value for p in candidates]
        next_val = azl.greedy_next_hop(self.value, neighbor_values, target_value)
        for p in candidates:
            if abs(p.value - next_val) < 1e-15:
                return p
        return None

    def deliver(self, msg: dict):
        self.inbox.append(msg)

class AZLMesh:
    def __init__(self):
        self.nodes: Dict[int, AZLNode] = {}

    def add_node(self, n: int) -> AZLNode:
        if n not in self.nodes:
            self.nodes[n] = AZLNode(n)
        return self.nodes[n]

    def link(self, a_n: int, b_n: int):
        a = self.add_node(a_n)
        b = self.add_node(b_n)
        a.add_peer(b)
        b.add_peer(a)

    def route(self, src_n: int, dst_n: int, payload=None, max_hops=32) -> dict:
        if src_n not in self.nodes or dst_n not in self.nodes:
            return {"delivered": False, "reason": "unknown_endpoint"}
        src = self.nodes[src_n]
        dst_val = self.nodes[dst_n].value
        cur = src
        path = [cur.n]
        visited = {cur.n}
        hops = 0
        while cur.n != dst_n and hops < max_hops:
            nxt = cur.route_next(dst_val, visited)
            if not nxt:
                return {"delivered": False, "path": path, "hops": hops, "reason": "stuck"}
            cur = nxt
            path.append(cur.n)
            visited.add(cur.n)
            hops += 1
        delivered = cur.n == dst_n
        if delivered:
            cur.deliver({"src": src_n, "dst": dst_n, "payload": payload, "hops": hops})
        return {"delivered": delivered, "path": path, "hops": hops, "src": src.address, "dst": self.nodes[dst_n].address}

def build_ring(ns: List[int]) -> AZLMesh:
    """Simple test topology: a ring + a few shortcuts"""
    m = AZLMesh()
    for n in ns:
        m.add_node(n)
    for i in range(len(ns)):
        m.link(ns[i], ns[(i+1) % len(ns)])
    # add 2-hop shortcuts for better greedy convergence
    for i in range(len(ns)):
        m.link(ns[i], ns[(i+2) % len(ns)])
    return m
