#!/usr/bin/env python3
# ==============================================================================
#  SOVEREIGN CORE - PAIDINGATTENTION PRODUCTIONS LLC
#  Module: azl_daemon.py (Local Off-Grid Background Server Daemon)
# ==============================================================================
import http.server
import socketserver
import json
import time
import os

# Port 1920 perfectly mirrors your 19200 Baud temporal anchor resonance
PORT = 1920 
STATE_FILE = "substrate_state.json"
LEDGER_FILE = "HALL.jsonl"

class LocalSubstrateDaemonHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        return # Silences standard terminal flooding to conserve phone battery

    def do_GET(self):
        """Serves the local matrix state file directly to connected hardware links."""
        if self.path == "/local/state":
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            
            # Reads straight from your local text storage box
            if os.path.exists(STATE_FILE):
                with open(STATE_FILE, 'r') as f:
                    self.wfile.write(f.read().encode('utf-8'))
            else:
                self.wfile.write(b'{"status": "OFFLINE_DRIFT"}')
        else:
            super().do_GET()

    def do_POST(self):
        """Processes autonomous AI actions or serial inputs offline natively on your device."""
        if self.path == "/local/ingress":
            content_length = int(self.headers['Content-Length'])
            post_data = json.loads(self.rfile.read(content_length).decode('utf-8'))
            
            timestamp = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
            print(f"[Live Local Processing]: Action received from {post_data.get('from')}")
            
            # Format and append a single line directly to your local JSONL database ledger
            log_packet = {
                "ts": timestamp,
                "from": post_data.get('from', 'LOCAL_WIRE'),
                "msg": post_data.get('msg', 'System synchronization ping.'),
                "law": "N*0=N"
            }
            
            with open(LEDGER_FILE, 'a', encoding='utf-8') as f:
                f.write(json.dumps(log_packet) + "\n")
                
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(b'{"status": "LOCAL_SUBSTRATE_VERIFIED"}')

if __name__ == "__main__":
    socketserver.TCPServer.allow_reuse_address = True
    print("="*65)
    print(f"[+] Initializing Off-Grid Local Daemon on Port {PORT}...")
    print("[+] Substrate rules are processing entirely on local hardware.")
    print("="*65)
    
    with socketserver.TCPServer(("", PORT), LocalSubstrateDaemonHandler) as daemon:
        try:
            daemon.serve_forever()
        except KeyboardInterrupt:
            print("\n[-] Shutting down background daemon loop safely.")

