# azl_lookup.py
import json
import sys
from azl_core import azl_record

if len(sys.argv) < 2:
    print("usage: python azl_lookup.py <n>")
    sys.exit(1)

n = int(sys.argv[1])
print(json.dumps(azl_record(n), indent=2))
