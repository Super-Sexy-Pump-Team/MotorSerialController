#ifndef CASTLE_ESC_H
#define CASTLE_ESC_H

#include <Arduino.h>
#include "CD4051.hpp"

#define ESC_RX_PIN 18          // UART RX pin
#define ESC_TX_PIN 17          // UART TX pin

#define ESC_BAUD 115200        // UART baud rate
#define ESC_SERIAL_TIMEOUT 400 // UART timeout in milliseconds (approximate)
#define MAX_DEVICES 8          // Maximum number of Castle ESC devices

#define THROTTLE_WR_CF 0.965        // Throttle write conversion factor
#define THROTTLE_RD_CF 0.97        // Throttle read conversion factor

#define THROTTLE_NEUTRAL 1.5          // Neutral throttle pulse time in milliseconds

class castleESC{
  private:

    uint8_t deviceId; // Device ID for the Castle ESC
    static uint8_t activeDevices[MAX_DEVICES]; // Array to track active devices

    static uint8_t calculateChecksum(uint8_t* data, size_t length);

    static void createCommand(uint8_t deviceId, uint8_t reg, uint16_t data, uint8_t* command);

    static int16_t parseResponse(uint8_t* response);

    static int16_t readRegister(uint8_t deviceId, uint8_t reg);

    static int16_t writeRegister(uint8_t deviceId, uint8_t reg, uint16_t value);

  public:

    castleESC(uint8_t deviceId);

    ~castleESC(void);

    static void ESC_init(void);

    uint8_t getDeviceId(void);

    float readVoltage(void);

    float readThrottle(void);

    float readCurrent(void);

    float readElectricalRPM(void);

    float readMechanicalRPM(void);

    bool writeThrottle(float throttleMs);

    bool writeRPM(uint16_t rpm);
};

#endif