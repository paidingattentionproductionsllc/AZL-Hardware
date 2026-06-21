// AZL Node 2 - Handheld Anchor
// LAT-MILTG6042BP / Github
// ESP32 / Arduino

#include <WiFi.h>
#include "azl_api.h"

// --- Set your WiFi ---
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_PASSWORD";

unsigned long last_heartbeat = 0;
const unsigned long HEARTBEAT_MS = 60000; // 60s

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== AZL Node 2 ===");
  Serial.printf("Node: %s / %s\n", AZL_NODE_ID, AZL_NODE_NAME);
  Serial.printf("Role: %s | Logic: %s\n", AZL_NODE_ROLE, AZL_NODE_LOGIC);
  Serial.printf("Manifest: %s | Tier %s | %s\n", AZL_MANIFEST_VERSION, AZL_MANIFEST_TIER, AZL_LAW);
  Serial.printf("Lattice: %lu addresses [%s -> %s]\n", AZL_TOTAL_ADDRESSES, AZL_FIRST_ADDRESS, AZL_LAST_ADDRESS);
  Serial.printf("Proof: %s | Domain: %s\n", AZL_PROOF, AZL_DOMAIN);

  // 1. Validate local manifest
  if (azl_validate_local_manifest()) {
    Serial.println("[AZL] Manifest OK - T1-6 immutable");
    Serial.printf("[AZL] Substrate: %s=%d verified, %s=%lld verified\n",
      AZL_EVENT_MIYAKE_ADDR, AZL_EVENT_MIYAKE_RESULT,
      AZL_EVENT_M87_ADDR, AZL_EVENT_M87_RESULT);
  } else {
    Serial.println("[AZL] Manifest MISMATCH - halt");
    while(1) delay(1000);
  }

  // 2. WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi OK: %s\n", WiFi.localIP().toString().c_str());

  // 3. Register with Lattice
  Serial.println("[AZL] Registering...");
  AZL_RegisterResult reg = azl_register_node();
  if (reg.success) {
    Serial.printf("[AZL] %s\n", reg.message.c_str());
    Serial.println("[AZL] Node anchored to the Absolute Zero Lattice.");
  } else {
    Serial.printf("[AZL] Register failed: %s\n", reg.message.c_str());
  }
  last_heartbeat = millis();
}

void loop() {
  unsigned long now = millis();
  if (now - last_heartbeat >= HEARTBEAT_MS) {
    last_heartbeat = now;
    Serial.println("[AZL] Heartbeat re-anchor...");
    AZL_RegisterResult reg = azl_register_node();
    Serial.printf("[AZL] %s | success=%d\n", reg.message.c_str(), reg.success);
    
    // Tier send stub - wire to real ingest when ready
    // AZL_TierPayload p = {0,0,0,0};
    // azl_send_tiers(p);
  }
  delay(100);
}
