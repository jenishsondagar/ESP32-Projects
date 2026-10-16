#include <Arduino.h>
#include "oled.h"
#include "animation.h"

static bool running = false;
static int currentAnimation = 0;
static int frame = 0;
static unsigned long lastFrameTime = 0;

static void drawSmiley() {
    display.clearDisplay();

    display.drawCircle(64, 32, 25, SSD1306_WHITE);

    if (frame % 2 == 0) {
        display.fillCircle(52, 25, 3, SSD1306_WHITE);
        display.fillCircle(76, 25, 3, SSD1306_WHITE);
    } else {
        display.drawLine(48, 25, 56, 25, SSD1306_WHITE);
        display.drawLine(72, 25, 80, 25, SSD1306_WHITE);
    }

    display.drawLine(49, 40, 54, 45, SSD1306_WHITE);
    display.drawLine(54, 45, 74, 45, SSD1306_WHITE);
    display.drawLine(74, 45, 79, 40, SSD1306_WHITE);

    display.display();
}

static void drawAngryFace() {
    display.clearDisplay();

    if (frame % 2 == 0) {
        display.fillTriangle(
            15, 15, 55, 25, 55, 35, SSD1306_WHITE
        );

        display.fillTriangle(
            113, 15, 73, 25, 73, 35, SSD1306_WHITE
        );

        display.fillCircle(42, 35, 5, SSD1306_WHITE);
        display.fillCircle(86, 35, 5, SSD1306_WHITE);
    } else {
        display.drawLine(20, 33, 55, 33, SSD1306_WHITE);
        display.drawLine(73, 33, 108, 33, SSD1306_WHITE);
    }

    display.drawLine(47, 49, 81, 49, SSD1306_WHITE);
    display.display();
}

void animationStart(int animationID) {
    currentAnimation = animationID;
    frame = 0;
    running = true;
    lastFrameTime = 0;
    animationUpdate();
}

void animationUpdate() {
    if (!running) {
        return;
    }

    if (millis() - lastFrameTime < 300) {
        return;
    }

    lastFrameTime = millis();
    frame++;

    if (currentAnimation == 0) {
        drawSmiley();
    } else {
        drawAngryFace();
    }
}

void animationStop() {
    running = false;
}