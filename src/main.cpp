#include <Arduino.h>
#include <stdexcept>

#include "castleESC.hpp"

// Global variables
String inputString = "";
bool stringComplete = false;
bool escapeSequenceEnabled = false;

castleESC esc1(0);

void setup() {

  // Initialize primary Serial for debugging
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Initialize the Castle ESCs
  castleESC::ESC_init();
  
}


void loop() {
  // Read and display ESC data
  if (escapeSequenceEnabled) Serial.println("\x1B[H---------------------------------------\x1B[K");

  if (escapeSequenceEnabled) Serial.print("\033[2;1H");
  Serial.print("Voltage: ");
  float voltage = 0.0;
  try{
    voltage = esc1.readVoltage();
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
    current = esc1.readCurrent();
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
    throttle = esc1.readThrottle();
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
    rpm = esc1.readRPM();
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
        esc1.writeThrottle(throttleMs);

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
