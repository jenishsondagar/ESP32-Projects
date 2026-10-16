#include <Arduino.h>
#include <WiFi.h>

#include "oled.h"
#include "boot.h"
#include "animation.h"
#include "wifi_manager.h"
#include "bluetooth_manager.h"

const char* menuItems[] = {
    "Smiley Head",
    "WiFi",
    "Bluetooth",
    "Angry Face",
    "WiFi Scanner",
    "Bluetooth Scanner"
};

const int MENU_COUNT = 6;

enum AppMode {
    MODE_MENU,
    MODE_ANIMATION,
    MODE_WIFI_SELECT,
    MODE_WIFI_PASSWORD,
    MODE_WIFI_CONNECTING,
    MODE_WIFI_RESULT,
    MODE_BT_SERVER,
    MODE_WIFI_SCANNER,
    MODE_BT_SCANNER,
    MODE_BT_ERROR
};

AppMode mode = MODE_MENU;

int selectedMenu = 0;
int wifiSelected = 0;
int btSelected = 0;

bool scanScreenShown = false;
bool btScanScreenShown = false;
bool btScanStarted = false;
bool btErrorShown = false;

bool wifiResultConnected = false;

unsigned long btServerStartTime = 0;

String chosenSSID;
String passwordBuffer;

// ---------------- MENU ----------------

void showMainMenu() {
    oledShowMenu(menuItems, MENU_COUNT, selectedMenu);
}

// ---------------- WIFI SCAN DISPLAY ----------------

void showWifiList(bool scannerMode) {
    int count = wifiNetworkCount();

    if (count <= 0) {
        oledShowStatus(
            "WIFI SCANNER",
            "No networks found",
            "Check WiFi router",
            "2x = Back"
        );
        return;
    }

    if (wifiSelected >= count) {
        wifiSelected = 0;
    }

    String name = wifiNetworkName(wifiSelected);
    String number = String(wifiSelected + 1)
                    + "/" + String(count);

    if (scannerMode) {
        oledShowStatus(
            "WIFI SCANNER",
            name.c_str(),
            number.c_str(),
            "1x next  2x back"
        );
    } else {
        oledShowStatus(
            "SELECT WIFI",
            name.c_str(),
            number.c_str(),
            "1x next  2x select"
        );
    }
}

// ---------------- BLUETOOTH SCAN DISPLAY ----------------

void showBluetoothList() {
    int total = btScanCount();
    int stored = btScanStoredCount();

    if (stored <= 0) {
        oledShowStatus(
            "BT SCANNER",
            "No devices found",
            ("Total: " + String(total)).c_str(),
            "2x = Back"
        );
        return;
    }

    if (btSelected >= stored) {
        btSelected = 0;
    }

    String name = btScanName(btSelected);
    String number = String(btSelected + 1)
                    + "/" + String(stored);

    oledShowStatus(
        "BT SCANNER",
        name.c_str(),
        ("Found: " + String(total)).c_str(),
        (number + "  1x next").c_str()
    );
}

// ---------------- EXIT TO MENU ----------------

void returnToMenu() {
    if (mode == MODE_ANIMATION) {
        animationStop();
    }

    if (mode == MODE_WIFI_SELECT ||
        mode == MODE_WIFI_PASSWORD ||
        mode == MODE_WIFI_CONNECTING ||
        mode == MODE_WIFI_RESULT ||
        mode == MODE_WIFI_SCANNER) {
        wifiStop();
    }

    if (mode == MODE_BT_SERVER) {
        btServerStop();
    }

    if (mode == MODE_BT_SCANNER) {
        btScanStop();
    }

    mode = MODE_MENU;
    scanScreenShown = false;
    btScanScreenShown = false;
    btErrorShown = false;

    showMainMenu();
}

// ---------------- ENTER SELECTED MENU ----------------

void enterMenu() {
    switch (selectedMenu) {

        case 0:
            mode = MODE_ANIMATION;
            animationStart(0);
            break;

        case 1:
            wifiSelected = 0;
            scanScreenShown = false;
            mode = MODE_WIFI_SELECT;

            wifiScanStart();

            oledShowStatus(
                "SCANNING WIFI",
                "Please wait...",
                "",
                "2x = Back"
            );
            break;

        case 2:
            mode = MODE_BT_SERVER;
            btServerStart();
            btServerStartTime = millis();
            btErrorShown = false;

            oledShowStatus(
                "BLUETOOTH",
                "Waiting for phone",
                "Open BT terminal",
                "2x = Back"
            );
            break;

        case 3:
            mode = MODE_ANIMATION;
            animationStart(1);
            break;

        case 4:
            wifiSelected = 0;
            scanScreenShown = false;
            mode = MODE_WIFI_SCANNER;

            wifiScanStart();

            oledShowStatus(
                "WIFI SCANNER",
                "Scanning...",
                "Please wait",
                "2x = Back"
            );
            break;

        case 5:
            btSelected = 0;
            btScanScreenShown = false;
            mode = MODE_BT_SCANNER;

            btScanStarted = btScanStart();

            oledShowStatus(
                "BT SCANNER",
                btScanStarted
                    ? "Scanning Classic BT"
                    : "Scan start failed",
                "Please wait...",
                "2x = Back"
            );
            break;
    }
}

// ---------------- BUTTON EVENTS ----------------

void handleButton(ButtonEvent event) {
    if (event == BUTTON_NONE) {
        return;
    }

    // Single press = next menu item
    if (event == BUTTON_SINGLE) {

        if (mode == MODE_MENU) {
            selectedMenu++;

            if (selectedMenu >= MENU_COUNT) {
                selectedMenu = 0;
            }

            showMainMenu();
        }

        else if (mode == MODE_WIFI_SELECT ||
                 mode == MODE_WIFI_SCANNER) {

            if (scanScreenShown &&
                wifiNetworkCount() > 0) {

                wifiSelected++;

                if (wifiSelected >= wifiNetworkCount()) {
                    wifiSelected = 0;
                }

                showWifiList(
                    mode == MODE_WIFI_SCANNER
                );
            }
        }

        else if (mode == MODE_BT_SCANNER) {

            if (btScanScreenShown &&
                btScanStoredCount() > 0) {

                btSelected++;

                if (btSelected >= btScanStoredCount()) {
                    btSelected = 0;
                }

                showBluetoothList();
            }
        }

        return;
    }

    // Double press = enter or exit
    if (event == BUTTON_DOUBLE) {

        if (mode == MODE_MENU) {
            enterMenu();
        }

        else if (mode == MODE_ANIMATION) {
            returnToMenu();
        }

        else if (mode == MODE_WIFI_SELECT) {

            if (!scanScreenShown) {
                returnToMenu();
                return;
            }

            if (wifiNetworkCount() <= 0) {
                returnToMenu();
                return;
            }

            chosenSSID = wifiNetworkName(wifiSelected);
            passwordBuffer = "";

            mode = MODE_WIFI_PASSWORD;

            Serial.println();
            Serial.println("WiFi selected:");
            Serial.println(chosenSSID);
            Serial.println(
                "Type WiFi password in Serial Monitor, "
                "then press Enter."
            );

            oledShowStatus(
                "WIFI PASSWORD",
                chosenSSID.c_str(),
                "Type in Serial",
                "Monitor 115200"
            );
        }

        else if (mode == MODE_WIFI_PASSWORD ||
                 mode == MODE_WIFI_CONNECTING ||
                 mode == MODE_WIFI_RESULT ||
                 mode == MODE_WIFI_SCANNER ||
                 mode == MODE_BT_SERVER ||
                 mode == MODE_BT_SCANNER ||
                 mode == MODE_BT_ERROR) {

            returnToMenu();
        }
    }
}

// ---------------- WIFI PASSWORD INPUT ----------------

void handleSerialPassword() {
    while (Serial.available() > 0 &&
           mode == MODE_WIFI_PASSWORD) {

        char c = Serial.read();

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            wifiConnectStart(
                chosenSSID,
                passwordBuffer
            );

            mode = MODE_WIFI_CONNECTING;

            oledShowStatus(
                "WIFI CONNECTING",
                chosenSSID.c_str(),
                "Please wait...",
                "2x = Cancel"
            );

            Serial.println("Connecting...");
            passwordBuffer = "";
            return;
        }

        if (c == '\b' || c == 127) {
            if (passwordBuffer.length() > 0) {
                passwordBuffer.remove(
                    passwordBuffer.length() - 1
                );
            }
        }

        else if (passwordBuffer.length() < 63) {
            passwordBuffer += c;
        }
    }
}

// ---------------- CONTINUOUS UPDATES ----------------

void updateApplication() {

    if (mode == MODE_ANIMATION) {
        animationUpdate();
    }

    // WiFi selection and WiFi scanner
    if (mode == MODE_WIFI_SELECT ||
        mode == MODE_WIFI_SCANNER) {

        if (!scanScreenShown) {
            WifiScanState state = wifiScanUpdate();

            if (state == WIFI_SCAN_DONE) {
                scanScreenShown = true;

                if (wifiNetworkCount() > 0) {
                    showWifiList(
                        mode == MODE_WIFI_SCANNER
                    );
                } else {
                    oledShowStatus(
                        "WIFI SCANNER",
                        "No networks found",
                        "Check WiFi router",
                        "2x = Back"
                    );
                }
            }

            else if (state == WIFI_SCAN_ERROR) {
                scanScreenShown = true;

                oledShowAngryError("WiFi");
            }
        }
    }

    // WiFi connection
    if (mode == MODE_WIFI_CONNECTING) {

        WifiConnectState state = wifiConnectUpdate();

        if (state == WIFI_CONNECTED) {
            mode = MODE_WIFI_RESULT;
            wifiResultConnected = true;

            String ip = WiFi.localIP().toString();

            oledShowStatus(
                "WIFI CONNECTED",
                chosenSSID.c_str(),
                ip.c_str(),
                "2x = Back"
            );

            Serial.println("WiFi connected!");
            Serial.println(ip);
        }

        else if (state == WIFI_CONNECT_FAILED) {
            mode = MODE_WIFI_RESULT;
            wifiResultConnected = false;

            oledShowAngryError("WiFi");

            Serial.println("WiFi connection failed!");
        }
    }

    // Bluetooth server / phone connection
    if (mode == MODE_BT_SERVER) {

        if (btServerConnected()) {
            btErrorShown = true;

            oledShowStatus(
                "BT CONNECTED",
                "Phone connected",
                "ESP32-OLED",
                "2x = Back"
            );
        }

        else if (!btErrorShown &&
                 millis() - btServerStartTime >= 10000) {

            btErrorShown = true;
            oledShowAngryError("Bluetooth");
        }
    }

    // Classic Bluetooth scanner
    if (mode == MODE_BT_SCANNER &&
        btScanStarted &&
        !btScanScreenShown) {

        if (btScanUpdate()) {
            btScanScreenShown = true;
            showBluetoothList();
        }
    }
}

// ---------------- SETUP ----------------

void setup() {
    Serial.begin(115200);

    oledBegin();
    bootBegin();

    showMainMenu();
}

// ---------------- LOOP ----------------

void loop() {
    ButtonEvent event = bootUpdate();

    handleButton(event);
    handleSerialPassword();
    updateApplication();
}