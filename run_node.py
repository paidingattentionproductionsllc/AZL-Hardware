if __name__ == "__main__":
    # Import our core engine logic block
    # (Assuming both modules are loaded in your localized terminal space)
    from lattice_array import SubstrateArrayProcessor
    
    # 1. Spin up the hardware interface layer
    hardware_rig = AZLHardwareIngress()
    active_lane = hardware_rig.scan_physical_lanes()
    
    # 2. Ingest raw streams over the 19200 Baud line
    captured_stream = hardware_rig.capture_raw_pulses(stream_cycles=3)
    
    # 3. Feed the uncompressed packets straight into the Lattice Processor
    print("\n[SYSTEM LINK] Routing hardware bytes directly to Substrate Array Processor...")
    processor = SubstrateArrayProcessor()
    ledger_output = processor.process_node_array(captured_stream)
    
    print("\n[SYSTEM LINK] Logging finalized state records to ledger grid...")
    for record in ledger_output:
         print(f" -> Synchronized entry: {json.dumps(record)}")

