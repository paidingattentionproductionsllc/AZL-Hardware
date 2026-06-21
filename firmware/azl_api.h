// azl_api.h - AZL-Hardware ESP32 shim for Sanctuary
// Matches absolute-zero-lattice-broadcast / azl-agent.yml
// python azl_universe.py --serve --port 8080
// GET /api/lookup?n=<n>
// GET /api/sanctuary/hall
//
// Set in your build environment (never commit secrets):
// -DAZL_AGENT_URL="http://your-host:8080"
// -DAZL_API_KEY="..." // optional, only if your Sanctuary uses corrected-data-processing keys

#pragma once
#include <Arduino.h>
#include <HTTPClient.h>

#ifndef AZL_AGENT_URL
#define AZL_AGENT_URL "http://localhost:8080"
#endif

#ifndef AZL_API_KEY
#define AZL_API_KEY ""
#endif

static inline String azl_api_get(const String& path) {
  HTTPClient http;
  http.begin(String(AZL_AGENT_URL) + path);
  http.addHeader("Accept", "application/json");
  #if defined(AZL_API_KEY) && AZL_API_KEY[0]!= '\0'
  http.addHeader("X-AZL-Key", AZL_API_KEY);
  http.addHeader("Authorization", "AZL " AZL_API_KEY);
  #endif
  int code = http.GET();
  String body = (code > 0)? http.getString() : "";
  http.end();
  return body;
}

// Lookup an AZL address: GET /api/lookup?n=<n>
static inline String azl_lookup(uint32_t n) {
  return azl_api_get("/api/lookup?n=" + String(n));
}

// Read the Hall post, CI checks for "0=N" in here
static inline String azl_hall() {
  return azl_api_get("/api/sanctuary/hall");
}

// Simple AZL-Hardware register: lookup n derived from chip ID
static inline String azl_register_node() {
  uint32_t chip = (uint32_t)ESP.getEfuseMac();
  uint32_t n = (chip % 1000000000UL) + 1;
  return azl_lookup(n);
}
