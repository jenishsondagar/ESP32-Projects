#include <Arduino.h>
#include <BluetoothSerial.h>
#include <string.h>
#include "bluetooth_manager.h"

#if !defined(CONFIG_BT_ENABLED) || \
    !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled for this board.
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Bluetooth Classic SPP is not available.
#endif

static BluetoothSerial SerialBT;

static bool isBtStarted = false;
static bool scanning = false;
static bool scanFinished = false;

static unsigned long scanStartTime = 0;
static const unsigned long scanDuration = 10000;

static const int MAX_STORED_DEVICES = 10;
static char deviceNames[MAX_STORED_DEVICES][24];

static int storedCount = 0;
static int totalFound = 0;

static portMUX_TYPE btMux = portMUX_INITIALIZER_UNLOCKED;

static void deviceFound(BTAdvertisedDevice* device) {
    if (device == nullptr) {
        return;
    }

   String name = String(device->getName().c_str());

    if (name.length() == 0) {
        name = device->getAddress().toString();
    }

    portENTER_CRITICAL(&btMux);

    totalFound++;

    if (storedCount < MAX_STORED_DEVICES) {
        strncpy(
            deviceNames[storedCount],
            name.c_str(),
            sizeof(deviceNames[storedCount]) - 1
        );

        deviceNames[storedCount]
            [sizeof(deviceNames[storedCount]) - 1] = '\0';

        storedCount++;
    }

    portEXIT_CRITICAL(&btMux);
}

bool btServerStart() {
    if (btStarted) {
        SerialBT.end();
        isBtStarted = false;
    }

    if (!SerialBT.begin("ESP32-OLED", false)) {
        return false;
    }

    isBtStarted = true;
    return true;
}

bool btServerConnected() {
    return btStarted && SerialBT.hasClient();
}

void btServerStop() {
    if (btStarted) {
        SerialBT.end();
     isBtStarted = false;
    }
}

bool btScanStart() {
    btScanStop();

    portENTER_CRITICAL(&btMux);
    storedCount = 0;
    totalFound = 0;
    portEXIT_CRITICAL(&btMux);

    scanFinished = false;

    if (!SerialBT.begin("ESP32-BT-SCANNER", true)) {
        scanFinished = true;
        return false;
    }
 isBtStarted = true;

    scanStartTime = millis();

    scanning = SerialBT.discoverAsync(deviceFound, scanDuration);

    if (!scanning) {
        scanFinished = true;
        return false;
    }

    return true;
}

bool btScanUpdate() {
    if (scanning &&
        millis() - scanStartTime >= scanDuration) {

        SerialBT.discoverAsyncStop();

        scanning = false;
        scanFinished = true;
    }

    return scanFinished;
}

void btScanStop() {
    if (scanning && btStarted) {
        SerialBT.discoverAsyncStop();
    }

    scanning = false;
    scanFinished = true;

    if (btStarted) {
        SerialBT.end();
       isBtStarted = false;
    }
}

int btScanCount() {
    int count;

    portENTER_CRITICAL(&btMux);
    count = totalFound;
    portEXIT_CRITICAL(&btMux);

    return count;
}

int btScanStoredCount() {
    int count;

    portENTER_CRITICAL(&btMux);
    count = storedCount;
    portEXIT_CRITICAL(&btMux);

    return count;
}

String btScanName(int index) {
    char copy[24] = {0};

    portENTER_CRITICAL(&btMux);

    if (index >= 0 && index < storedCount) {
        strncpy(copy, deviceNames[index], sizeof(copy) - 1);
    }

    portEXIT_CRITICAL(&btMux);

    return String(copy);
}
