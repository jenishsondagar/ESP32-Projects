#include "boot.h"

#define BOOT_BUTTON 0

static bool lastReading = HIGH;
static bool stableState = HIGH;

static unsigned long debounceTime = 0;
static unsigned long lastClickTime = 0;

static const unsigned long debounceDelay = 40;
static const unsigned long doubleClickDelay = 350;

static int clickCount = 0;

void bootBegin() {
    pinMode(BOOT_BUTTON, INPUT_PULLUP);
}

ButtonEvent bootUpdate() {
    bool reading = digitalRead(BOOT_BUTTON);

    if (reading != lastReading) {
        debounceTime = millis();
    }

    if (millis() - debounceTime >= debounceDelay) {
        if (reading != stableState) {
            stableState = reading;

            if (stableState == LOW) {
                clickCount++;
                lastClickTime = millis();
            }
        }
    }

    lastReading = reading;

    if (clickCount > 0 &&
        millis() - lastClickTime >= doubleClickDelay) {

        int count = clickCount;
        clickCount = 0;

        if (count >= 2) {
            return BUTTON_DOUBLE;
        }

        return BUTTON_SINGLE;
    }

    return BUTTON_NONE;
}