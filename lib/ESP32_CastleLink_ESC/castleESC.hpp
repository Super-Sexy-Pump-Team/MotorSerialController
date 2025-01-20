#ifndef CASTLE_ESC_H
#define CASTLE_ESC_H

#include <Arduino.h>

#define SERIAL2_RX_PIN 18  // UART RX pin
#define SERIAL2_TX_PIN 17  // UART TX pin
#define SERIAL_BAUD 115200 // UART baud rate
#define SERIAL_TIMEOUT 500 // UART timeout in milliseconds
#define DEVICE_ID 0       // Castle ESC device ID

// Function prototypes
uint8_t calculateChecksum(uint8_t* data, size_t length);
void createCommand(uint8_t deviceId, uint8_t reg, uint16_t data, uint8_t* command);
int16_t parseResponse(uint8_t* response);
int16_t readRegister(uint8_t deviceId, uint8_t reg);
int16_t writeRegister(uint8_t deviceId, uint8_t reg, uint16_t value);
float readVoltage(uint8_t deviceId);
float readThrottle(uint8_t deviceId);
float readCurrent(uint8_t deviceId);
float readRPM(uint8_t deviceId);
bool writeThrottle(uint8_t deviceId, float throttleMs);

#endif