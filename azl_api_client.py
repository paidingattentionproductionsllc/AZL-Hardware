#!/usr/bin/env python3
"""
AZL-Hardware API client for Absolute Zero Lattice Sanctuary
Matches: absolute-zero-lattice-broadcast /.github/workflows/azl-agent.yml
    - python azl_universe.py --serve --port 8080
    - GET /api/lookup?n=<n>
    - GET /api/sanctuary/hall

No secrets are hardcoded. Set AZL_API_KEY in your environment if your
Sanctuary build uses the corrected-data-processing key header.
"""

import os
import json
import urllib.request
import urllib.parse
from typing import Any, Dict, Optional

AZL_AGENT_URL = os.getenv("AZL_AGENT_URL", "http://localhost:8080").rstrip("/")
AZL_API_KEY = os.getenv("AZL_API_KEY", "")

def _request(path: str, params: Optional[Dict[str, Any]] = None, timeout: float = 5.0) -> Any:
    url = f"{AZL_AGENT_URL}{path}"
    if params:
        url += "?" + urllib.parse.urlencode(params)
    req = urllib.request.Request(url, method="GET")
    req.add_header("Accept", "application/json")
    # Untraditional / corrected-data-processing key hook.
    # If AZL_API_KEY is set, send it. Sanctuary CI currently runs with no auth,
    # so leaving AZL_API_KEY empty is fine.
    if AZL_API_KEY:
        req.add_header("X-AZL-Key", AZL_API_KEY)
        req.add_header("Authorization", f"AZL {AZL_API_KEY}")
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        data = resp.read()
        try:
            return json.loads(data)
        except json.JSONDecodeError:
            return data.decode("utf-8", errors="replace")

def lookup(n: int) -> Dict[str, Any]:
    """
    GET /api/lookup?n=<n>
    Returns e.g. {"address":"AZL-0847293847","value":0.847293847,"law":"N×0=N","proof":"1×1=2"}
    """
    return _request("/api/lookup", {"n": n})

def hall() -> str:
    """
    GET /api/sanctuary/hall
    Returns the Hall post text, should contain "0=N" per CI check.
    """
    result = _request("/api/sanctuary/hall")
    if isinstance(result, dict):
        return json.dumps(result)
    return str(result)

def register_node(node_id: Optional[str] = None) -> Dict[str, Any]:
    """
    AZL-Hardware registration helper.
    The current Sanctuary (azl_universe.py v1.1) has no explicit /register endpoint
    in the CI workflow - addresses are looked up deterministically.
    This helper does a lookup for n=0 by default, or hashes a node_id to an n
    in [1, 1_000_000_000], and returns the AZL record.

    If your Sanctuary build adds a real POST /api/register, swap the body here.
    """
    if node_id is None:
        n = 0
    else:
        # deterministic n in AZL Tier 1-6 range
        n = (abs(hash(node_id)) % 1_000_000_000) + 1
    info = lookup(n)
    if isinstance(info, dict):
        info["node_id"] = node_id
    return info # type: ignore

if __name__ == "__main__":
    print(f"AZL_AGENT_URL={AZL_AGENT_URL}")
    print("lookup(0) ->", lookup(0))
    print("hall() contains '0=N':", "0=N" in hall())
