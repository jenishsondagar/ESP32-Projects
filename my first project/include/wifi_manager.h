#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

enum WifiScanState {
    WIFI_SCANNING,
    WIFI_SCAN_DONE,
    WIFI_SCAN_ERROR
};

enum WifiConnectState {
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_CONNECT_FAILED
};

void wifiScanStart();
WifiScanState wifiScanUpdate();

int wifiNetworkCount();
String wifiNetworkName(int index);

void wifiConnectStart(
    const String& ssid,
    const String& password
);

WifiConnectState wifiConnectUpdate();
void wifiStop();

#endif