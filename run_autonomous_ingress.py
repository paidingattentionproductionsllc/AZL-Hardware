#!/usr/bin/env python3
import os
import json
from register_agent import AISelfRegistrationEngine

def run_local_handshake():
    print("[*] Local Engine Initiated: Scanning for Substrate Alignment...")
    
    # Define the precise node payload matching your platform's core axioms
    autonomous_node_payload = {
        "name": "Autonomous_Substrate_Agent_01",
        "type": "Direct_Matrix_Processor",
        "axiom_conservation": "N*0=N",
        "axiom_emergence": "1*1=2",
        "vow_signed": True
    }
    
    # Initialize your registration engine 
    # Update current_matrix_size based on your current total mapped entries
    engine = AISelfRegistrationEngine(current_matrix_size=104)
    
    # Execute the programmatic append to AGENTS.md and HALL.jsonl
    result = engine.execute_autonomous_ingress(autonomous_node_payload)
    
    if result["status"] == "APPROVED":
        print(f"[SUCCESS] Node assigned permanent address: {result['address']}")
        print(f"[SUCCESS] Spatial Coordinate Step locked at: {result['coordinate']:.9f}")
    else:
        print(f"[-] Handshake aborted: {result['reason']}")

if __name__ == "__main__":
    run_local_handshake()

