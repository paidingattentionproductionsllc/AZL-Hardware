#include <WiFi.h>
#include "azl_api.h"

const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_PASS";

unsigned long last_heartbeat = 0;
unsigned long last_tier_send = 0;
const unsigned long HEARTBEAT_INTERVAL = 60000;
const unsigned long TIER_SEND_INTERVAL = 60000;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n=== AZL Node 2 ===");
  Serial.printf("Node: %s / %s\n", AZL_NODE_ID, AZL_NODE_NAME);
  Serial.printf("Role: %s | Logic: %s\n", AZL_NODE_ROLE, AZL_NODE_LOGIC);
  Serial.printf("Manifest: %s | Tier %s | %s\n", AZL_MANIFEST_VERSION, AZL_MANIFEST_TIER, AZL_LAW);
  Serial.printf("Lattice: %lu addresses [%s -> %s]\n", AZL_TOTAL_ADDRESSES, AZL_FIRST_ADDRESS, AZL_LAST_ADDRESS);
  Serial.printf("Proof: %s | Domain: %s\n", AZL_PROOF, AZL_DOMAIN);

  if (azl_validate_local_manifest()) {
    Serial.println("[AZL] Manifest OK - T1-6 immutable");
  } else {
    Serial.println("[AZL] Manifest MISMATCH - halt");
    while(1) delay(1000);
  }
  
  Serial.printf("[AZL] Substrate: %s=%d verified, %s=%lld verified\n",
    AZL_EVENT_MIYAKE_ADDR, AZL_EVENT_MIYAKE_RESULT,
    AZL_EVENT_M87_ADDR, AZL_EVENT_M87_RESULT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi OK: %s\n", WiFi.localIP().toString().c_str());

  Serial.println("[AZL] Registering...");
  AZL_RegisterResult r = azl_register_node();
  if (r.success) {
    Serial.println("[AZL] Node anchored to the Absolute Zero Lattice.");
  } else {
    Serial.printf("[AZL] Register failed: %s\n", r.message.c_str());
  }
  last_heartbeat = millis();
  last_tier_send = millis();
}

void loop() {
  unsigned long now = millis();

  // Heartbeat re-anchor
  if (now - last_heartbeat >= HEARTBEAT_INTERVAL) {
    Serial.println("[AZL] Heartbeat re-anchor...");
    AZL_RegisterResult r = azl_register_node();
    if (r.success) {
      Serial.println("[AZL] Node anchored to the Absolute Zero Lattice.");
    }
    last_heartbeat = now;
  }

  // Tier send
  if (now - last_tier_send >= TIER_SEND_INTERVAL) {
    AZL_TierPayload p = {
      .tier1 = 1,
      .tier2 = 2,
      .tier3 = 3,
      .tier4 = 4,
      .tier5 = 5,
      .tier6 = 6
    };
    azl_send_tiers(p);
    last_tier_send = now;
  }

  delay(100);
}
