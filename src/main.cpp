#include <Arduino.h>
#include <stdexcept>

#include "castleESC.hpp"

// Global variables
String inputString = "";
bool stringComplete = false;
bool escapeSequenceEnabled = false;

void setup() {
  // Initialize primary Serial for debugging
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Initialize Serial2 for ESC communication
  Serial2.begin(SERIAL_BAUD, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN);

  // Ensure the throttle is set to neutral (1.5ms) at startup (this should prevent motor movement)
  try {
    writeThrottle(DEVICE_ID, 1.5);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle to neutral: " + String(e.what()));
  }

  // Send 5 0x00 bytes to clear the command buffer and synchronize with the ESC
  uint8_t clearBuffer[] = {0x00, 0x00, 0x00, 0x00, 0x00};
  Serial2.write(clearBuffer, sizeof(clearBuffer));  // Send the 5 zero bytes

  // Clear receive buffer
  while (Serial2.available()) Serial2.read();

  Serial.println("ESP32-S3 ESC Controller initialized and command buffer cleared");
  
}


void loop() {
  // Read and display ESC data
  if (escapeSequenceEnabled) Serial.println("\x1B[H---------------------------------------\x1B[K");

  if (escapeSequenceEnabled) Serial.print("\033[2;1H");
  Serial.print("Voltage: ");
  float voltage = 0.0;
  try{
    voltage = readVoltage(DEVICE_ID);
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.printf("%.2fV\n", voltage);
  } catch (std::runtime_error& e) {
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.println("Failed to read voltage: " + String(e.what()));
  }

  if (escapeSequenceEnabled) Serial.print("\033[3;1H");
  Serial.print("Current: ");
  float current = 0.0;
  try{
    current = readCurrent(DEVICE_ID);
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.printf("%.2fA\n", current);
  } catch (std::runtime_error& e) {
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.println("Failed to read current: " + String(e.what()));
  }

  float power = voltage * current;
  if (escapeSequenceEnabled) Serial.print("\033[4;1H");
  Serial.print("Power: ");
  if (escapeSequenceEnabled) Serial.print("\x1B[K");
  Serial.printf("%.2fW\n\r", power);

  if (escapeSequenceEnabled) Serial.print("\033[5;1H");
  Serial.print("Throttle: ");
  float throttle = 0.0;
  try{
    throttle = readThrottle(DEVICE_ID);
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.printf("%.2fms\n", throttle);
  } catch (std::runtime_error& e) {
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.println("Failed to read throttle: " + String(e.what()));
  }

  if (escapeSequenceEnabled) Serial.print("\033[6;1H");
  Serial.print("RPM: ");
  float rpm = 0.0;
  try{
    rpm = readRPM(DEVICE_ID);
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.printf("%.0f\n", rpm);
  } catch (std::runtime_error& e) {
    if (escapeSequenceEnabled) Serial.print("\x1B[K");
    Serial.println("Failed to read RPM: " + String(e.what()));
  }

  if (escapeSequenceEnabled) Serial.print("\033[7;1H");
  Serial.print("---------------------------------------");
  if (escapeSequenceEnabled) Serial.print("\033[K");
  Serial.println();
  
  // Check for serial input
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') { // Check for newline or carriage return (enter key)
      stringComplete = true;
      if (escapeSequenceEnabled) Serial.print("\033[8;1H\033[1mSet Throttle Pulse (ms): \033[0m\033[K\033[s");
    } else if (inChar == 'e') { // Check for 'e' key to enable escape sequences
      escapeSequenceEnabled = true;
      Serial.print("\033[2J\033[0m\033[8;1H\033[1mSet Throttle Pulse (ms): \033[0m\033[s\033[H");
      Serial.println("Serial escape sequences enabled");
    } else if (inChar == 'd') { // Check for 'd' key to disable escape sequences
      escapeSequenceEnabled = false;
      Serial.println("Serial escape sequences disabled");
    } else {
      // Append the character to the input string
      inputString += inChar;

      // Echo the character back to the terminal
      if (escapeSequenceEnabled) Serial.print("\033[u\033[5m");
      Serial.print(inChar);
      if (escapeSequenceEnabled) Serial.print("\033[0m\033[s");
    }
  }

  // Process throttle command if the enter button was pressed
  if (stringComplete) {
    float throttleMs = inputString.toFloat();
    if (throttleMs >= 1.0 && throttleMs <= 2.0) {
      try {
        writeThrottle(DEVICE_ID, throttleMs);

        if (escapeSequenceEnabled) Serial.print("\033[9;1H\033[K\033[32m");
        Serial.println("Throttle set to " + String(throttleMs) + "ms");
        if (escapeSequenceEnabled) Serial.print("\033[0m");

      } catch (std::runtime_error& e) {
        if (escapeSequenceEnabled) Serial.print("\033[9;1H\033[K\033[31m");
        Serial.println("Failed to set throttle: " + String(e.what()));
        if (escapeSequenceEnabled) Serial.print("\033[0m");
      }
    } else {
      if (escapeSequenceEnabled) Serial.print("\033[9;1H\033[K\033[31m");
      Serial.println("Invalid throttle value (must be between 1.0 and 2.0)");
      if (escapeSequenceEnabled) Serial.print("\033[0m");
    }
    
    inputString = "";
    stringComplete = false;
  }

  // Use the delay() to control the frequency of loop()
  // Min delay(10) for 100 Hz
  delay(1000);
}
