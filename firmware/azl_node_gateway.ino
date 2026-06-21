#include <WiFi.h>
#include <esp_now.h>
#include <HTTPClient.h>
#include "azl_api.h"

extern "C" {
#include "azl_space.h"
#include "azl_mesh.h"
}

// -------- Node config --------
#ifndef AZL_NODE_ID
#define AZL_NODE_ID 1
#endif

// Gateway build for Node 1
#define AZL_GATEWAY 1

// Sanctuary / Lattice WiFi
#ifndef AZL_WIFI_SSID
#define AZL_WIFI_SSID "ABSOLUTE_ZERO_LATTICE"
#endif
#ifndef AZL_WIFI_PASS
#define AZL_WIFI_PASS "1x1=2_CERTAINTY"
#endif

#ifndef AZL_AGENT_URL
#define AZL_AGENT_URL "http://10.143.50.1:8080"
#endif

static const uint8_t AZL_BROADCAST[6] = {0xFF,0xFF};

// -------- AZL packet --------
typedef struct __attribute__((packed)) {
  uint32_t src_addr;
  uint32_t dst_addr;
  uint16_t seq;
  uint8_t ttl;
  uint8_t payload_len;
  uint8_t payload[64];
} azl_packet_t;

static uint16_t tx_seq = 0;

// TODO: AZL - map your mesh neighbor table to ESP-NOW peers
static void azl_add_peer(const uint8_t *mac) {
  if (esp_now_is_peer_exist(mac)) return;
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = 1;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

// TODO: AZL - replace with your actual next-hop lookup
static uint32_t azl_next_hop_addr(uint32_t dst) {
#ifdef AZL_MESH_NEXT_HOP
  return AZL_MESH_NEXT_HOP(dst);
#else
  return dst;
#endif
}

// TODO: AZL - map AZL address -> ESP-NOW MAC
static bool azl_addr_to_mac(uint32_t azl_addr, uint8_t *mac_out) {
  struct { uint32_t addr; uint8_t mac[6]; } map[] = {
    {1, {0x24,0x6F,0x28,0xAA,0xBB,0x01}},
    {2, {0x24,0x6F,0x28,0xAA,0xBB,0x02}},
    {3, {0x24,0x6F,0x28,0xAA,0xBB,0x03}},
  };
  for (auto &e : map) {
    if (e.addr == azl_addr) { memcpy(mac_out, e.mac, 6); return true; }
  }
  return false;
}

static void azl_forward_packet(azl_packet_t *pkt) {
  if (pkt->ttl == 0) return;
  pkt->ttl--;

  uint32_t my_addr = AZL_NODE_ID;
  uint32_t dst = pkt->dst_addr;

  if (dst == my_addr) {
    Serial.printf("[AZL] delivered locally, seq %u, %u bytes\n", pkt->seq, pkt->payload_len);
    return;
  }

  uint32_t next_hop = azl_next_hop_addr(dst);
  uint8_t next_mac[6];
  if (!azl_addr_to_mac(next_hop, next_mac)) {
    Serial.printf("[AZL] no MAC for next_hop %lu\n", (unsigned long)next_hop);
    return;
  }

  azl_add_peer(next_mac);
  esp_err_t r = esp_now_send(next_mac, (uint8_t*)pkt, sizeof(azl_packet_t));
  Serial.printf("[AZL] fwd %lu -> %lu via %lu : %s\n",
    (unsigned long)pkt->src_addr, (unsigned long)dst, (unsigned long)next_hop,
    r == ESP_OK? "ok" : "fail");
}

// -------- ESP-NOW callbacks --------
void on_esp_now_recv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len < (int)sizeof(azl_packet_t)) return;
  azl_packet_t pkt;
  memcpy(&pkt, data, sizeof(pkt));
  Serial.printf("[RX] %lu -> %lu seq %u ttl %u\n",
    (unsigned long)pkt.src_addr, (unsigned long)pkt.dst_addr, pkt.seq, pkt.ttl);
  azl_forward_packet(&pkt);

#if AZL_GATEWAY
  // Bridge mesh packet up to Sanctuary
  if (WiFi.status() == WL_CONNECTED && pkt.payload_len > 0) {
    char payload_str[65] = {0};
    memcpy(payload_str, pkt.payload, pkt.payload_len > 64? 64 : pkt.payload_len);
    HTTPClient http;
    String url = String(AZL_AGENT_URL) + "/ingest";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String body = "{\"node\":" + String(AZL_NODE_ID) + ",\"src\":" + String((unsigned long)pkt.src_addr) + ",\"payload\":\"" + String(payload_str) + "\"}";
    int code = http.POST(body);
    Serial.printf("[BRIDGE] fwd to Sanctuary: %d\n", code);
    http.end();
  }
#endif
}

void on_esp_now_sent(const uint8_t *mac, esp_now_send_status_t status) {
  // optional: log acks
}

// -------- setup / loop --------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nAZL Node %d booting%s\n", AZL_NODE_ID,
#if AZL_GATEWAY
  " [GATEWAY]"
#else
  ""
#endif
  );

  // --- AZL Sanctuary register ---
  if (strlen(AZL_WIFI_SSID) > 0) {
    Serial.println("[AZL] connecting to Sanctuary AP for register...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(AZL_WIFI_SSID, AZL_WIFI_PASS);
    unsigned long t0 = millis();
    while (WiFi.status()!= WL_CONNECTED && millis() - t0 < 8000) { delay(250); Serial.print("."); }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      String resp = azl_register_node();
      Serial.print("[AZL] Sanctuary response: ");
      Serial.println(resp);
    } else {
      Serial.println("[AZL] Sanctuary register skipped, WiFi timeout");
    }
#if AZL_GATEWAY
    Serial.print("[AZL] Gateway WiFi stay-up, IP: ");
    Serial.println(WiFi.localIP());
#else
    WiFi.disconnect(true, true);
    delay(100);
#endif
  }
  // --- end register ---

  WiFi.mode(WIFI_AP_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init()!= ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1) delay(1000);
  }
  esp_now_register_recv_cb(on_esp_now_recv);
  esp_now_register_send_cb(on_esp_now_sent);

  // TODO: AZL - init your mesh: azl_mesh_init(AZL_NODE_ID);
  Serial.println("[AZL] mesh ready");
}

void loop() {
  // Example beacon every 5s
  static uint32_t last = 0;
  if (millis() - last > 5000) {
    last = millis();
    azl_packet_t pkt = {};
    pkt.src_addr = AZL_NODE_ID;
    pkt.dst_addr = 2; // Node 2
    pkt.seq = tx_seq++;
    pkt.ttl = 8;
    const char *msg = "N*0=N";
    pkt.payload_len = strlen(msg);
    memcpy(pkt.payload, msg, pkt.payload_len);
    azl_forward_packet(&pkt);
  }
  delay(10);
}
