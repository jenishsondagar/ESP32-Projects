#ifndef BOOT_H
#define BOOT_H

#include <Arduino.h>

enum ButtonEvent {
    BUTTON_NONE,
    BUTTON_SINGLE,
    BUTTON_DOUBLE
};

void bootBegin();
ButtonEvent bootUpdate();

#endif