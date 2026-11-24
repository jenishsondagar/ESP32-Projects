#pragma once
#include <Arduino.h>

// Same channel must be used by Server and every Client.
#define ESPNOW_MAGIC 0x544D5031
#define ESPNOW_CHANNEL 1
#define MAX_CLIENTS 10

#define MSG_SCAN        1
#define MSG_DISCOVERY   2
#define MSG_COMMAND     3
#define MSG_TEMPERATURE 4

// One packet carries both DHT11 sensor readings.
struct __attribute__((packed)) ESPNowPacket {
    uint32_t magic;
    uint8_t type;
    uint8_t clientID;   // Server-assigned slot for display; 0 from clients
    uint8_t selected;
    uint8_t reserved;
    float temperature1;
    float humidity1;
    float temperature2;
    float humidity2;
    uint32_t counter;
};
