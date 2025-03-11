#include <Arduino.h>
#include <stdexcept>
#include <optional>

#include "castleESC.hpp"


HardwareSerial* castleESC::_serialDevice = &Serial2; // Serial device used for ESC communication
uint8_t castleESC::activeDevices[MAX_DEVICES] = {0}; // Array to track active devices


/**
 * @brief Castle ESC object constructor
 * @param deviceId Device ID of the Castle ESC
 * @retval castleESC
 */
castleESC::castleESC(uint8_t deviceId) {
  if (deviceId > MAX_DEVICES - 1) {
    throw std::runtime_error("Invalid device ID");
  }
  if (this->activeDevices[deviceId] == 1) {
    throw std::runtime_error("Device ID already in use");
  }

  this->deviceId = deviceId;
  this->activeDevices[deviceId] = 1;
}


/**
 * @brief Castle ESC object destructor
 * @retval void
 */
castleESC::~castleESC(void) {
  this->activeDevices[this->deviceId] = 0;
}


/**
 * @brief Initialize all active Castle ESC controllers
 * @param serialDevice Pointer to Serial device to use for ESC communication
 * @param baud Baud rate for the serial communication
 * @param rxPin RX pin for the serial communication
 * @param txPin TX pin for the serial communication
 * @param muxInh MUX INH pin
 * @param muxA MUX A pin
 * @param muxB MUX B pin
 * @param muxC MUX C pin
 * @retval void
 */
void castleESC::ESC_init(HardwareSerial *serialDevice, long baud, uint8_t rxPin, uint8_t txPin, uint8_t muxInh, uint8_t muxA, uint8_t muxB, uint8_t muxC) {

  castleESC::_serialDevice = serialDevice;

  // Initialize Serial for ESC communication
  _serialDevice->begin(baud, SERIAL_8N1, rxPin, txPin);
  // Initialize the MUX
  MUX_init(muxInh, muxA, muxB, muxC);

  // Send 5 0x00 bytes to clear the command buffer and synchronize with the ESC
  uint8_t clearBuffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
  _serialDevice->write(clearBuffer, sizeof(clearBuffer));  // Send the 5 zero bytes

  // Set the throttle of all active devices to neutral (1.5ms)
  for (int i = 0; i < MAX_DEVICES; i++) {
    if (activeDevices[i] == 1) {
      uint16_t throttleValue = (uint16_t)((THROTTLE_NEUTRAL - 1.0) * 65535);
      try {
        writeRegister(i, 128, throttleValue);
      } catch (std::runtime_error& e) {
        Serial.println("Failed to set throttle of device #" + String(i) + " to neutral: " + String(e.what()));
      }
    }
  }

  // Clear receive buffer
  while (_serialDevice->available()) _serialDevice->read();

  Serial.println("ESC Controllers Initialized");
}


/**
  * @brief Calculate the checksum for a given data array
  * @param data Data array to calculate the checksum for
  * @param length Length of the data array
  * @retval uint8_t
  */
uint8_t castleESC::calculateChecksum(uint8_t* data, size_t length) {
  uint16_t sum = 0;
  for (size_t i = 0; i < length; i++) {
    sum += data[i];  // Sum the first four bytes (ignoring the checksum byte)
  }
  
  // Now calculate the checksum as modular sum 0 - bytes
  return (256 - (sum % 256));
}


/**
  * @brief Create a command to send to the Castle ESC
  * @param deviceId Device ID of the Castle ESC
  * @param reg Register address to write to
  * @param data Data to write to the register
  * @param command Buffer to save the command
  * @retval void
  */
void castleESC::createCommand(uint8_t deviceId, uint8_t reg, uint16_t data, uint8_t* command) {
  command[0] = 0x80 | (deviceId & 0x3F);      // Byte 0: Device ID
  command[1] = reg;                           // Byte 1: Register address
  command[2] = (data >> 8) & 0xFF;            // Byte 2: Write Data[15:8]
  command[3] = data & 0xFF;                   // Byte 3: Write Data[7:0]
  command[4] = calculateChecksum(command, 4); // Byte 4: Checksum
}


/**
  * @brief Parse the response from the Castle ESC
  * @param response Buffer holding the response from the Castle ESC
  * @retval int16_t
  */
int16_t castleESC::parseResponse(uint8_t* response) {
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
  * @param deviceId Device ID of the Castle ESC
  * @param reg Register address to read from
  * @retval int16_t
  */
int16_t castleESC::readRegister(uint8_t deviceId, uint8_t reg) {
  uint8_t command[5];
  uint8_t response[3];

  MUX_select(deviceId);
  while (_serialDevice->available()) _serialDevice->read();  // Clear receive buffer
  
  // Create the command and send it to the ESC
  createCommand(deviceId, reg, 0, command);
  _serialDevice->write(command, 5);
  
  uint16_t timeout = 0;
  // Wait for the full response
  while (_serialDevice->available() < 3){
    delay(1);
    timeout++;
    
    // If the response takes too long, throw a timeout error
    if (timeout > ESC_SERIAL_TIMEOUT) {
      MUX_disable();
      throw std::runtime_error("Read Timeout");
      return -1;
    }
  }

  MUX_disable();

  // Read the response from the ESC
  _serialDevice->readBytes(response, 3);
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
  * @param deviceId Device ID of the Castle ESC
  * @param reg Register address to write to
  * @param value Value to write to the register
  * @retval int16_t
  */
int16_t castleESC::writeRegister(uint8_t deviceId, uint8_t reg, uint16_t value) {
  uint8_t command[5];
  uint8_t response[3];

  MUX_select(deviceId);
  while (_serialDevice->available()) _serialDevice->read();  // Clear receive buffer

  // Create the command and send it to the ESC  
  createCommand(deviceId, reg, value, command);
  _serialDevice->write(command, 5);
  
  uint16_t timeout = 0;
  // Wait for the full response
  while (_serialDevice->available() < 3){
    delay(1);
    timeout++;

    // If the response takes too long, throw a timeout error
    if (timeout > ESC_SERIAL_TIMEOUT) {
      MUX_disable();
      throw std::runtime_error("Write Response Timeout");
      return -1;
    }
  }

  MUX_disable();

  // Read the response from the ESC
  _serialDevice->readBytes(response, 3);
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
  * @param void
  * @retval float
  */
float castleESC::readVoltage(void) {
  // Read the voltage register (0x00)
  int16_t value = readRegister(this->deviceId, 0);
  if (value >= 0) {
    // Convert the 16-bit value to a voltage (0-100V)
    return (float)value / 2042.0 * 20.0;
  }
  return -1.0;
}


/**
  * @brief Read the throttle pulse time from the Castle ESC
  * @param void
  * @retval float
  */
float castleESC::readThrottle(void) {
  // Read the throttle register (0x03)
  int16_t value = readRegister(this->deviceId, 3);
  if (value >= 0) {
    // Convert the 16-bit value to a throttle pulse time (1-2ms)
    return (float)value / 2042.0 * 1.0 * THROTTLE_RD_CF;
  }
  return -1.0;
}


/**
  * @brief Read the motor current from the Castle ESC
  * @param void
  * @retval float
  */
float castleESC::readCurrent(void) {
  // Read the current register (0x02)
  int16_t value = readRegister(this->deviceId, 2);
  if (value >= 0) {
    // Convert the 16-bit value to a current (0-250A)
    return (float)value / 2042.0 * 50.0;
  }
  return -1.0;
}


/**
  * @brief Read the motor's electrical RPM from the Castle ESC
  * @param void
  * @retval float
  */
float castleESC::readElectricalRPM(void) {
  // Read the RPM register (0x05)
  int16_t value = readRegister(this->deviceId, 5);
  if (value >= 0) {
    // Convert the 16-bit value to an RPM (0-20416.66)
    return (float)value / 2042.0 * 20416.66;
  }
  return -1.0;
}


/**
  * @brief Read the motor's mechanical RPM from the Castle ESC
  * @param void
  * @retval float
  */
float castleESC::readMechanicalRPM(void) {
  float electrical = readElectricalRPM();
  return electrical / 2.0;
}


/**
  * @brief Write a throttle pulse time to the Castle ESC
  * @param throttleMs Throttle pulse time in milliseconds (1-2ms)
  * @retval bool
  */
bool castleESC::writeThrottle(float throttleMs) {
  // Convert the throttle pulse time to a 16-bit value (0-65535)
  uint32_t throttleValue = (uint32_t)((throttleMs - 1.0) * 65535 * THROTTLE_WR_CF);
  // Write the throttle value to the ESC
  if (throttleValue <= 65535){
    return writeRegister(this->deviceId, 128, throttleValue) >= 0;
  } else {
    return writeRegister(this->deviceId, 128, 65535) >= 0;
  }
}


/**
  * @brief Set the RPM of the motor using a PID
  * @param rpm_SP RPM setpoint
  * @retval bool
  */
bool castleESC::setRPM(uint16_t rpm_SP) {
  float P, I, D;   // Proportional, Integral, and Derivative terms
  float err = 100000;    // Error between desired and actual RPM
  float err_prev = 0;    // Previous error value
  float output;    // Output value for the PID controller

  int8_t Kp = 30;  // Proportional gain
  float Ki = 0.01; // Integral gain
  int8_t Kd = 0; // Derivative gain
  
  float dt = 10;  // Time step (ms)

  while (std::abs(err) > 10) {
    try{
      // Calculate the error between the desired and actual RPM
      err = (float) rpm_SP - readElectricalRPM();
    } catch (std::runtime_error& e) {
      Serial.println("Failed to read RPM: " + String(e.what()));
      return false;
    }
    
    /* Proportional */
    P = Kp * err;

    /* Integral */
    I += Ki * err * dt;

    /* Derivative */
    D = Kd * (err - err_prev) / dt;

    // Calculate the throttle value
    output = (P + I + D)/10000000 + 1.55;

    Serial.println("\033[13;1HUnclamped Output: \033[K" + String(output, 4) + "ms");
    
    if (output > 2.0) {
      output = 1.9;
    } else if (output < 1.5){
      output = 1.51;
    }

    try{
      // Write the throttle value to the ESC
      Serial.println("\033[14;1HTrying to set throttle to \033[K" + String(output, 4) + "ms");
      Serial.printf("\033[15;1HCurrent Error: \033[K%.2f\n", err);
      writeThrottle(output);
    } catch (std::runtime_error& e) {
      Serial.println("\033[20Failed to set RPM: " + String(e.what()));
      return false;
    }

    err_prev = err;
    delay(dt);
  }

  return true;
}


uint8_t castleESC::getDeviceId(void){
  return this->deviceId;
}