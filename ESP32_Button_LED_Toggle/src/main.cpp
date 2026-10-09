#include <Arduino.h>

#define BUTTON_PIN 4
#define LED_PIN 5

bool ledState = false;
bool lastButtonState = HIGH;

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    Serial.println("System Ready");
}

void loop()
{
    bool buttonState = digitalRead(BUTTON_PIN);

    
    if (lastButtonState == HIGH && buttonState == LOW){
         ledState = !ledState;
         digitalWrite(LED_PIN, ledState);

        if (ledState)
        {
            Serial.println("LED ON");
        }
        else
        {
            Serial.println("LED OFF");
        }
        delay(200);
    }

    lastButtonState = buttonState;
}