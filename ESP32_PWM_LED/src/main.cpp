#include <Arduino.h>

const int ledPin = 16; 

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop(){
  
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    analogWrite(ledPin, dutyCycle);
    delay(15);
  }

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    analogWrite(ledPin, dutyCycle);
    delay(15);
  }
}
