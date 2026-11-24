#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <string.h>

#include "espnow_protocol.h"

// =====================================================
// ESP32-C3 CLIENT: upload the SAME sketch to every unit
// DHT11 #1 DATA = GPIO 4
// DHT11 #2 DATA = GPIO 5
// OLED SDA = GPIO 6, SCL = GPIO 7
// =====================================================

#define DHT1_PIN 4
#define DHT2_PIN 5
#define DHT_TYPE DHT11

#define OLED_SDA 6
#define OLED_SCL 7
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C

#define WIFI_CHANNEL ESPNOW_CHANNEL

DHT dht1(DHT1_PIN, DHT_TYPE);
DHT dht2(DHT2_PIN, DHT_TYPE);

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

static uint8_t serverMAC[6] = {0};
static bool serverKnown = false;
static bool serverPeerReady = false;
static bool clientSelected = false;
static bool oledReady = false;

static float t1 = NAN, h1 = NAN, t2 = NAN, h2 = NAN;
static uint32_t packetCounter = 0;
static uint32_t lastDHTRead = 0;
static uint32_t lastSend = 0;
static uint32_t lastOLED = 0;

static portMUX_TYPE rxMux = portMUX_INITIALIZER_UNLOCKED;
static bool scanResponsePending = false;
static bool commandPending = false;
static bool pendingSelected = false;
static uint8_t pendingServerMAC[6] = {0};

static bool sameMAC(const uint8_t *a, const uint8_t *b) {
    return memcmp(a, b, 6) == 0;
}

static bool addServerPeer(const uint8_t *mac) {
    if (esp_now_is_peer_exist(mac)) return true;

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = WIFI_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    return esp_now_add_peer(&peer) == ESP_OK;
}

static void readSensors() {
    // DHT11 should not be sampled too frequently.
    if (millis() - lastDHTRead < 2000) return;
    lastDHTRead = millis();

    float a = dht1.readTemperature();
    float b = dht1.readHumidity();
    float c = dht2.readTemperature();
    float d = dht2.readHumidity();

    if (!isnan(a)) t1 = a;
    if (!isnan(b)) h1 = b;
    if (!isnan(c)) t2 = c;
    if (!isnan(d)) h2 = d;
}

static void sendPacket(uint8_t type) {
    if (!serverKnown || !serverPeerReady) return;
    if (type == MSG_TEMPERATURE && !clientSelected) return;

    ESPNowPacket packet = {};
    packet.magic = ESPNOW_MAGIC;
    packet.type = type;
    packet.clientID = 0; // No manually configured client ID
    packet.selected = clientSelected ? 1 : 0;
    packet.temperature1 = t1;
    packet.humidity1 = h1;
    packet.temperature2 = t2;
    packet.humidity2 = h2;
    packet.counter = packetCounter++;

    esp_now_send(serverMAC, (const uint8_t *)&packet, sizeof(packet));
}

// Arduino-ESP32 Core 2.x callback signature (same style as original project).
static void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
    if (len != (int)sizeof(ESPNowPacket)) return;

    ESPNowPacket packet;
    memcpy(&packet, data, sizeof(packet));
    if (packet.magic != ESPNOW_MAGIC) return;

    if (packet.type == MSG_SCAN) {
        portENTER_CRITICAL(&rxMux);
        memcpy(pendingServerMAC, mac, 6);
        scanResponsePending = true;
        portEXIT_CRITICAL(&rxMux);
        return;
    }

    if (packet.type == MSG_COMMAND) {
        // Commands are unicast. Accept only from the Server learned via scan.
        if (!serverKnown || !sameMAC(mac, serverMAC)) return;

        portENTER_CRITICAL(&rxMux);
        pendingSelected = packet.selected != 0;
        commandPending = true;
        portEXIT_CRITICAL(&rxMux);
    }
}

static void processReceivedWork() {
    bool doScanReply = false;
    bool doCommand = false;
    bool newSelected = false;
    uint8_t mac[6] = {0};

    portENTER_CRITICAL(&rxMux);
    if (scanResponsePending) {
        memcpy(mac, pendingServerMAC, 6);
        scanResponsePending = false;
        doScanReply = true;
    }
    if (commandPending) {
        newSelected = pendingSelected;
        commandPending = false;
        doCommand = true;
    }
    portEXIT_CRITICAL(&rxMux);

    if (doScanReply) {
        memcpy(serverMAC, mac, 6);
        serverKnown = true;
        serverPeerReady = addServerPeer(serverMAC);
        if (serverPeerReady) {
            // Reply immediately to the Server's 5-second scan.
            sendPacket(MSG_DISCOVERY);
        }
    }

    if (doCommand) {
        clientSelected = newSelected;
        Serial.println(clientSelected ? "CLIENT SELECTED" : "CLIENT DESELECTED");
    }
}

static void updateOLED() {
    if (!oledReady) return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("ESP32-C3 DUAL DHT11");
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    display.setCursor(0, 15);
    display.print("Server: ");
    display.println(serverKnown ? "CONNECTED" : "WAITING");

    display.setCursor(0, 27);
    display.print("Status: ");
    display.println(clientSelected ? "SELECTED" : "AVAILABLE");

    display.setCursor(0, 40);
    display.print("S1 T:");
    if (isnan(t1)) display.print("--");
    else display.print(t1, 1);
    display.print("C H:");
    if (isnan(h1)) display.print("--");
    else display.print(h1, 0);
    display.println("%");

    display.setCursor(0, 53);
    display.print("S2 T:");
    if (isnan(t2)) display.print("--");
    else display.print(t2, 1);
    display.print("C H:");
    if (isnan(h2)) display.print("--");
    else display.print(h2, 0);
    display.println("%");

    display.display();
}

void setup() {
    Serial.begin(115200);
    delay(500);

    dht1.begin();
    dht2.begin();

    Wire.begin(OLED_SDA, OLED_SCL);
    oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (oledReady) {
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(0, 20);
        display.println("DUAL DHT11 CLIENT");
        display.display();
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

    Serial.print("Client MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW INIT FAILED");
        while (true) delay(1000);
    }

    esp_now_register_recv_cb(onDataRecv);
    Serial.println("CLIENT READY - WAITING FOR SERVER SCAN");
}

void loop() {
    processReceivedWork();
    readSensors();

    // Selected clients stream updated readings every 2 seconds.
    if (clientSelected && millis() - lastSend >= 2000) {
        lastSend = millis();
        sendPacket(MSG_TEMPERATURE);
    }

    if (millis() - lastOLED >= 500) {
        lastOLED = millis();
        updateOLED();
    }

    delay(5);
}
