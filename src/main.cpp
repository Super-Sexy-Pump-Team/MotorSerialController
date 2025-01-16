#include <Arduino.h>

// Constants
const uint8_t SERIAL2_RX_PIN = 18;  // Define your RX pin
const uint8_t SERIAL2_TX_PIN = 17;  // Define your TX pin
const uint32_t SERIAL_BAUD = 115200;
const uint8_t DEVICE_ID = 0;

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

// Global variables
String inputString = "";
bool stringComplete = false;


void setup() {
  // Initialize primary Serial for debugging
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Initialize Serial2 for ESC communication
  Serial2.begin(SERIAL_BAUD, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN);

  // Ensure the throttle is set to neutral (1.5ms) at startup (this should prevent motor movement)
  writeThrottle(DEVICE_ID, 1.5);  // Set throttle to 1.5ms (neutral)

  // Clear any pending data
  while (Serial2.available()) Serial2.read();

  // Send 5 0x00 bytes to clear the command buffer and synchronize with the ESC
  uint8_t clearBuffer[] = {0x00, 0x00, 0x00, 0x00, 0x00};
  Serial2.write(clearBuffer, sizeof(clearBuffer));  // Send the 5 zero bytes

  Serial.println("ESP32-S3 ESC Controller initialized and command buffer cleared");
}


void loop() {
  // Read and display ESC data
  float voltage = readVoltage(DEVICE_ID);
  float current = readCurrent(DEVICE_ID);
  float throttle = readThrottle(DEVICE_ID);
  float rpm = readRPM(DEVICE_ID);
  float power = voltage * current;

  // Print readings
  Serial.printf("Voltage: %.2fV\n", voltage);
  Serial.printf("Current: %.2fA\n", current);
  Serial.printf("Power: %.2fW\n", power);
  Serial.printf("Throttle: %.2fms\n", throttle);
  Serial.printf("RPM: %.0f\n", rpm);
  
  // Check for serial input
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }

  // Process throttle command if received
  if (stringComplete) {
    float throttleMs = inputString.toFloat();
    if (throttleMs >= 1.0 && throttleMs <= 2.0) {
      if (writeThrottle(DEVICE_ID, throttleMs)) {
        Serial.printf("Throttle set to: %.2fms\n", throttleMs);
      } else {
        Serial.println("Failed to set throttle");
      }
    } else {
      Serial.println("Invalid throttle value (must be between 1.0 and 2.0)");
    }
    
    inputString = "";
    stringComplete = false;
  }

  Serial.println("------------------------");
  // Use the delay() to control the frequency of loop()
  // Min delay(10) for 100 Hz
  delay(1000);
}


uint8_t calculateChecksum(uint8_t* data, size_t length) {
  uint16_t sum = 0;
  for (size_t i = 0; i < length; i++) {
    sum += data[i];  // Sum the first four bytes (ignoring the checksum byte)
  }
  
  // Now calculate the checksum as modular sum 0 - bytes
  return (256 - (sum % 256));
}

void createCommand(uint8_t deviceId, uint8_t reg, uint16_t data, uint8_t* command) {
  command[0] = 0x80 | (deviceId & 0x3F);
  command[1] = reg;
  command[2] = (data >> 8) & 0xFF;
  command[3] = data & 0xFF;
  command[4] = calculateChecksum(command, 4);
}

int16_t parseResponse(uint8_t* response) {
  uint8_t checksum = calculateChecksum(response, 2);
  if (checksum != response[2]) return -1;
  return (response[0] << 8) | response[1];
}

int16_t readRegister(uint8_t deviceId, uint8_t reg) {
  uint8_t command[5];
  uint8_t response[3];
  
  createCommand(deviceId, reg, 0, command);
  Serial2.write(command, 5);
  
  if (Serial2.available() >= 3) {
    Serial2.readBytes(response, 3);
    return parseResponse(response);
  }
  return -1;
}

int16_t writeRegister(uint8_t deviceId, uint8_t reg, uint16_t value) {
  uint8_t command[5];
  uint8_t response[3];
  
  createCommand(deviceId, reg, value, command);
  Serial2.write(command, 5);
  
  if (Serial2.available() >= 3) {
    Serial2.readBytes(response, 3);
    return parseResponse(response);
  }
  return -1;
}

float readVoltage(uint8_t deviceId) {
  int16_t value = readRegister(deviceId, 0);
  if (value >= 0) {
    return (float)value / 2042.0 * 20.0;
  }
  return -1.0;
}

float readThrottle(uint8_t deviceId) {
  int16_t value = readRegister(deviceId, 3);
  if (value >= 0) {
    return (float)value / 2042.0 * 1.0;
  }
  return -1.0;
}

float readCurrent(uint8_t deviceId) {
  int16_t value = readRegister(deviceId, 2);
  if (value >= 0) {
    return (float)value / 2042.0 * 50.0;
  }
  return -1.0;
}

float readRPM(uint8_t deviceId) {
  int16_t value = readRegister(deviceId, 5);
  if (value >= 0) {
    return (float)value / 2042.0 * 20416.66;
  }
  return -1.0;
}

bool writeThrottle(uint8_t deviceId, float throttleMs) {
  uint16_t throttleValue = (uint16_t)((throttleMs - 1.0) * 65535);
  return writeRegister(deviceId, 128, throttleValue) >= 0;
}
