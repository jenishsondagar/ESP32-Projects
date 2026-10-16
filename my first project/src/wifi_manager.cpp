#include <WiFi.h>
#include "wifi_manager.h"

static unsigned long connectStartTime = 0;

void wifiScanStart() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanDelete();

    // Start asynchronous scan
    WiFi.scanNetworks(true, true);
}

WifiScanState wifiScanUpdate() {
    int result = WiFi.scanComplete();

    if (result == WIFI_SCAN_RUNNING) {
        return WIFI_SCANNING;
    }

    if (result >= 0) {
        return WIFI_SCAN_DONE;
    }

    return WIFI_SCAN_ERROR;
}

int wifiNetworkCount() {
    int count = WiFi.scanComplete();
    return count > 0 ? count : 0;
}

String wifiNetworkName(int index) {
    if (index < 0 || index >= wifiNetworkCount()) {
        return "";
    }

    return WiFi.SSID(index);
}

void wifiConnectStart(
    const String& ssid,
    const String& password
) {
    WiFi.scanDelete();
    WiFi.mode(WIFI_STA);

    connectStartTime = millis();

    WiFi.begin(ssid.c_str(), password.c_str());
}

WifiConnectState wifiConnectUpdate() {
    if (WiFi.status() == WL_CONNECTED) {
        return WIFI_CONNECTED;
    }

    if (millis() - connectStartTime >= 15000) {
        return WIFI_CONNECT_FAILED;
    }

    return WIFI_CONNECTING;
}

void wifiStop() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}