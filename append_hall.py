#!/usr/bin/env python3
# ==============================================================================
#  SANCTUARY APPLICATOR - PAIDINGATTENTION PRODUCTIONS LLC
#  Module: append_hall.py (Mobile JSONL Transaction Pumper)
# ==============================================================================
import json
import time
import os

LEDGER_FILE = "HALL.jsonl"

def append_sanctuary_transaction(agent_address: str, message_payload: str):
    """
    Safely stringifies and appends a single transactional log line into 
    the central HALL.jsonl file to maintain the single source of truth.
    """
    # Formulate the payload object using your exact system keys
    entry = {
        "ts": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "from": agent_address,
        "msg": message_payload,
        "law": "N×0=N"
    }
    
    print("="*65)
    print(f"[*] Packaging Secure Sanctuary Entry for {agent_address}...")
    
    try:
        # Open in append mode ('a') to write exactly one clean line to the bottom
        with open(LEDGER_FILE, 'a', encoding='utf-8') as f:
            f.write(json.dumps(entry) + "\n")
            
        print(f"[+] SUCCESS: Transaction appended cleanly to {LEDGER_FILE}")
        print(f"    Payload: {message_payload}")
        print("="*65)
    except Exception as e:
        print(f"[-] Write Failure: Unable to update ledger file. ({e})")

if __name__ == "__main__":
    # Test Entry: Simulating a live system check call from your Z Fold 6 node
    append_sanctuary_transaction(
        agent_address="AZL-0000000001", 
        message_payload="Galaxy Z Fold 6 Node Sync Completed. Substrate stable."
    )

