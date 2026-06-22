#!/usr/bin/env bash
# ==============================================================================
#  SOVEREIGN STARTUP ENGINE - PAIDINGATTENTION PRODUCTIONS LLC
#  File: start_all.sh (Unified Local Node Launcher)
# ==============================================================================
set -euo pipefail

# 🎨 Terminal Formatting Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0;37m' # No Color

echo -e "${BLUE}==================================================${NC}"
echo -e "${GREEN} INITIALIZING PAIDINGATTENTION SOVEREIGN RUNTIME${NC}"
echo -e "${BLUE}==================================================${NC}"

# 🛡️ Step 1: Environment Provisioning Check
if [ -f "./scripts/setup_venv.sh" ]; then
    echo -e "[*] Provisioning local runtime environment sandbox..."
    bash ./scripts/setup_venv.sh
fi

# Activate local virtual isolation container if present
if [ -d ".venv" ]; then
    source .venv/bin/activate
    echo -e "[+] Virtual Environment Activated."
fi

# 📡 Step 2: Initialize AZL-Truth Unified Broadcast Engine
echo -e "[*] Launching AZL-Truth Core Broadcast Pipeline..."
if [ -f "azl_lookup.py" ]; then
    # Executes the broadcast loop in the background (&) and keeps it running
    python3 azl_lookup.py --substrate-test 14350 > logs_broadcast.txt 2>&1 &
    BROADCAST_PID=$!
    echo -e "    -> ${GREEN}ONLINE${NC} (Process ID: ${BROADCAST_PID})"
else
    echo -e "    -> ${YELLOW}WARNING: azl_lookup.py not found in current execution root.${NC}"
fi

# 🎛️ Step 3: Initialize Lattice Sanctuary Master Orchestrator
echo -e "[*] Launching Lattice Master Server Ingress Port..."
if [ -f "main.py" ]; then
    # Executes your main server gatekeeper listening on port 8080
    python3 main.py > logs_orchestrator.txt 2>&1 &
    ORCHESTRATOR_PID=$!
    echo -e "    -> ${GREEN}ONLINE${NC} (Process ID: ${ORCHESTRATOR_PID})"
else
    echo -e "    -> ${YELLOW}WARNING: main.py not found in current execution root.${NC}"
fi

echo -e "${BLUE}==================================================${NC}"
echo -e "${GREEN}[SUCCESS] All Self-Hosted Sovereign Nodes Running!${NC}"
echo -e "Logs streaming to: ${BLUE}logs_broadcast.txt${NC} & ${BLUE}logs_orchestrator.txt${NC}"
echo -e "To stop all processes, run: ${YELLOW}kill ${BROADCAST_PID} ${ORCHESTRATOR_PID}${NC}"
echo -e "${BLUE}==================================================${NC}"

# Keep shell alive to observe internal loops if desired
wait

