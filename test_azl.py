from azl_core import azl_value, azl_address
assert azl_value(1) == 1e-9
assert azl_value(500_000_000) == 0.5
assert azl_value(1_000_000_000) == 1.0
assert azl_address(847293847) == "AZL-0847293847"
print("AZL T1-6 PASS – N×0=N")
