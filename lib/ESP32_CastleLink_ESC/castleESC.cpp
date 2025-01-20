#include <Arduino.h>
#include <stdexcept>

#include "castleESC.hpp"

/**
  * @brief Calculate the checksum for a given data array
  * @retval uint8_t
  */
uint8_t calculateChecksum(uint8_t* data, size_t length) {
  uint16_t sum = 0;
  for (size_t i = 0; i < length; i++) {
    sum += data[i];  // Sum the first four bytes (ignoring the checksum byte)
  }
  
  // Now calculate the checksum as modular sum 0 - bytes
  return (256 - (sum % 256));
}


/**
  * @brief Create a command to send to the Castle ESC
  * @retval void
  */
void createCommand(uint8_t deviceId, uint8_t reg, uint16_t data, uint8_t* command) {
  command[0] = 0x80 | (deviceId & 0x3F);      // Byte 0: Device ID
  command[1] = reg;                           // Byte 1: Register address
  command[2] = (data >> 8) & 0xFF;            // Byte 2: Write Data[15:8]
  command[3] = data & 0xFF;                   // Byte 3: Write Data[7:0]
  command[4] = calculateChecksum(command, 4); // Byte 4: Checksum
}


/**
  * @brief Parse the response from the Castle ESC
  * @retval int16_t
  */
int16_t parseResponse(uint8_t* response) {
  // Verify the checksum
  uint8_t checksum = calculateChecksum(response, 2);
  if (checksum != response[2]){
    throw std::runtime_error("Checksum mismatch");
  }
  // Combine the two bytes into a single 16-bit value
  return (response[0] << 8) | response[1];
}


/**
  * @brief Read a register from the Castle ESC
  * @retval int16_t
  */
int16_t readRegister(uint8_t deviceId, uint8_t reg) {
  uint8_t command[5];
  uint8_t response[3];

  while (Serial2.available()) Serial2.read();  // Clear receive buffer
  
  // Create the command and send it to the ESC
  createCommand(deviceId, reg, 0, command);
  Serial2.write(command, 5);
  
  uint16_t timeout = 0;
  // Wait for the full response
  while (Serial2.available() < 3){
    delay(1);
    timeout++;
    
    // If the response takes too long, throw a timeout error
    if (timeout > SERIAL_TIMEOUT) {
      throw std::runtime_error("Read Timeout");
      return -1;
    }
  }

  // Read the response from the ESC
  Serial2.readBytes(response, 3);
  int16_t responseVal = parseResponse(response);

  if (responseVal != 0xFFFF) { // Check for invalid response (corrupted data or wrong register)
    return responseVal;
  }
  else {
    // If the response is invalid, throw an error
    throw std::runtime_error("Invalid response data");
    return -1;
  }
}


/**
  * @brief Write a register to the Castle ESC
  * @retval int16_t
  */
int16_t writeRegister(uint8_t deviceId, uint8_t reg, uint16_t value) {
  uint8_t command[5];
  uint8_t response[3];

  while (Serial2.available()) Serial2.read();  // Clear receive buffer

  // Create the command and send it to the ESC  
  createCommand(deviceId, reg, value, command);
  Serial2.write(command, 5);
  
  uint16_t timeout = 0;
  // Wait for the full response
  while (Serial2.available() < 3){
    delay(1);
    timeout++;

    // If the response takes too long, throw a timeout error
    if (timeout > SERIAL_TIMEOUT) {
      throw std::runtime_error("Write Response Timeout");
      return -1;
    }
  }

  // Read the response from the ESC
  Serial2.readBytes(response, 3);
  int16_t responseVal = parseResponse(response);

  // Response data for write commands can be ignored
  // still return the response value for debugging
  if (responseVal != 0xFFFF) { // Check for invalid response (corrupted data or wrong register)
    return responseVal;
  }
  else {
    // If the response is invalid, throw an error
    throw std::runtime_error("Write Error");
    return -1;
  }
}


/**
  * @brief Read the motor voltage from the Castle ESC
  * @retval float
  */
float readVoltage(uint8_t deviceId) {
  // Read the voltage register (0x00)
  int16_t value = readRegister(deviceId, 0);
  if (value >= 0) {
    // Convert the 16-bit value to a voltage (0-20V)
    return (float)value / 2042.0 * 20.0;
  }
  return -1.0;
}


/**
  * @brief Read the throttle pulse time from the Castle ESC
  * @retval float
  */
float readThrottle(uint8_t deviceId) {
  // Read the throttle register (0x03)
  int16_t value = readRegister(deviceId, 3);
  if (value >= 0) {
    // Convert the 16-bit value to a throttle pulse time (1-2ms)
    return (float)value / 2042.0 * 1.0;
  }
  return -1.0;
}


/**
  * @brief Read the motor current from the Castle ESC
  * @retval float
  */
float readCurrent(uint8_t deviceId) {
  // Read the current register (0x02)
  int16_t value = readRegister(deviceId, 2);
  if (value >= 0) {
    // Convert the 16-bit value to a current (0-50A)
    return (float)value / 2042.0 * 50.0;
  }
  return -1.0;
}


/**
  * @brief Read the motor RPM from the Castle ESC
  * @retval float
  */
float readRPM(uint8_t deviceId) {
  // Read the RPM register (0x05)
  int16_t value = readRegister(deviceId, 5);
  if (value >= 0) {
    // Convert the 16-bit value to an RPM (0-20416.66)
    return (float)value / 2042.0 * 20416.66;
  }
  return -1.0;
}


/**
  * @brief Write a throttle pulse time to the Castle ESC
  * @retval bool
  */
bool writeThrottle(uint8_t deviceId, float throttleMs) {
  // Convert the throttle pulse time to a 16-bit value (0-65535)
  uint16_t throttleValue = (uint16_t)((throttleMs - 1.0) * 65535);
  // Write the throttle value to the ESC
  return writeRegister(deviceId, 128, throttleValue) >= 0;
}