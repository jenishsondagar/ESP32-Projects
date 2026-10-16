#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <Arduino.h>

bool btServerStart();
bool btServerConnected();
void btServerStop();

bool btScanStart();
bool btScanUpdate();
void btScanStop();

int btScanCount();
int btScanStoredCount();
String btScanName(int index);

#endif