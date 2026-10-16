#include "oled.h"

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);

void oledBegin() {
    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(
            SSD1306_SWITCHCAPVCC,
            OLED_ADDR)) {
        Serial.println("OLED failed");
        while (true) {
            delay(100);
        }
    }

    display.clearDisplay();
    display.display();
}

void oledShowMenu(
    const char* items[],
    int count,
    int selected
) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    int previous = (selected - 1 + count) % count;
    int next = (selected + 1) % count;

    display.setCursor(22, 8);
    display.print(items[previous]);

    // Fixed arrow
    display.setCursor(2, 28);
    display.print("<-");

    // Selected name
    display.setCursor(22, 28);
    display.print(items[selected]);

    display.setCursor(22, 48);
    display.print(items[next]);

    display.display();
}

void oledShowStatus(
    const char* title,
    const char* line1,
    const char* line2,
    const char* line3
) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print(title);

    display.setCursor(0, 16);
    display.print(line1);

    display.setCursor(0, 32);
    display.print(line2);

    display.setCursor(0, 48);
    display.print(line3);

    display.display();
}

void oledShowAngryError(const char* service) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Angry eyes
    display.drawLine(8, 12, 30, 20, SSD1306_WHITE);
    display.drawLine(30, 20, 39, 13, SSD1306_WHITE);

    display.drawLine(57, 13, 66, 20, SSD1306_WHITE);
    display.drawLine(66, 20, 88, 12, SSD1306_WHITE);

    display.drawLine(39, 26, 57, 26, SSD1306_WHITE);

    display.setCursor(0, 32);
    display.print("NOT CONNECTED");

    display.setCursor(0, 44);
    display.print("Please connect");

    display.setCursor(0, 55);
    display.print("your ");
    display.print(service);

    display.display();
}