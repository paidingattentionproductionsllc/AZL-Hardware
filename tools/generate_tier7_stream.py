#!/usr/bin/env python3
"""
generate_tier7_stream.py - AZL Tier 7 streaming generator
for paidingattentionproductionsllc / AZL-Hardware

Outputs sharded gzipped jsonl, never checks data into git.
Matches Lattice manifest: value = n * 1e-9
  law = "N×0=N", proof = "1×1=2"
"""

import argparse
import gzip
import hashlib
import json
import sys
from pathlib import Path

AZL_SCALE = 1e-9
AZL_LAW = "N×0=N"
AZL_PROOF = "1×1=2"

def azl_record(n: int) -> dict:
    return {
        "address": f"AZL-{n:010d}",
        "n": n,
        "value": n * AZL_SCALE,
        "law": AZL_LAW,
        "proof": AZL_PROOF
    }

def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(8<<20), b''):
            h.update(chunk)
    return h.hexdigest()

def write_shard(start_n: int, end_n: int, out_path: Path) -> dict:
    count = 0
    tmp_path = out_path.with_suffix(out_path.suffix + '.tmp')
    with gzip.open(tmp_path, 'wt', encoding='utf-8', compresslevel=6) as gz:
        for n in range(start_n, end_n):
            gz.write(json.dumps(azl_record(n), separators=(',', ':')))
            gz.write(chr(10))
            count += 1
    tmp_path.replace(out_path)
    return {
        "shard": out_path.name,
        "start_n": start_n,
        "end_n": end_n - 1,
        "count": count,
        "bytes": out_path.stat().st_size,
        "sha256": sha256_file(out_path)
    }

def main():
    ap = argparse.ArgumentParser(description='AZL Tier 7 streaming generator')
    ap.add_argument('--start', type=int, default=1)
    ap.add_argument('--end', type=int, default=1_000_000_000, help='exclusive, use 10000000001 for full Tier 7 10B')
    ap.add_argument('--out_dir', type=Path, default=Path('./azl_tier7'))
    ap.add_argument('--shard_size', type=int, default=50_000_000)
    ap.add_argument('--prefix', default='azl_tier7')
    args = ap.parse_args()

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = out_dir / 'manifest.json'
    if manifest_path.exists():
        with open(manifest_path) as f:
            manifest = json.load(f)
    else:
        manifest = {"tier": 7, "scale": AZL_SCALE, "law": AZL_LAW, "proof": AZL_PROOF, "shards": []}
    
    existing = {s["shard"]: s for s in manifest["shards"]}
    new_shards = []
    n = args.start
    shard_idx = 0
    while n < args.end:
        shard_end = min(n + args.shard_size, args.end)
        shard_name = f"{args.prefix}_{n:011d}_{shard_end-1:011d}.jsonl.gz"
        shard_path = out_dir / shard_name
        if shard_path.exists() and shard_name in existing:
            print(f"[{shard_idx}] skip {shard_name}", file=sys.stderr)
            new_shards.append(existing[shard_name])
        else:
            print(f"[{shard_idx}] writing {shard_name} n={n}..{shard_end-1}", file=sys.stderr, flush=True)
            info = write_shard(n, shard_end, shard_path)
            print(f"  -> {info['count']} records, {info['bytes']/(1024*1024):.1f} MB", file=sys.stderr)
            new_shards.append(info)
            manifest["shards"] = sorted(new_shards + [s for k, s in existing.items() if k not in {x["shard"] for x in new_shards}], key=lambda x: x["start_n"])
            with open(manifest_path, 'w') as f:
                json.dump(manifest, f, indent=2)
        n = shard_end
        shard_idx += 1

    manifest["shards"] = new_shards
    manifest["start_n"] = args.start
    manifest["end_n"] = args.end - 1
    manifest["total_records"] = sum(s["count"] for s in new_shards)
    with open(manifest_path, 'w') as f:
        json.dump(manifest, f, indent=2)
    print(f"Done. {manifest['total_records']} records in {len(new_shards)} shards", file=sys.stderr)

if __name__ == '__main__':
    main()
