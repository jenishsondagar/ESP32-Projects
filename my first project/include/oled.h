#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

#define OLED_SDA 21
#define OLED_SCL 22

extern Adafruit_SSD1306 display;

void oledBegin();

void oledShowMenu(
    const char* items[],
    int count,
    int selected
);

void oledShowStatus(
    const char* title,
    const char* line1,
    const char* line2,
    const char* line3
);

void oledShowAngryError(const char* service);

#endif