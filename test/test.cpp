#include <Arduino.h>
#include <stdexcept>

#include "castleESC.hpp"


castleESC motor1(1);
castleESC motor2(0);

void setup() {
  // Initialize primary Serial for debugging
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Initialize the Castle ESCs
  castleESC::ESC_init();

};

void loop(){
  try {
    motor1.writeThrottle(1.5);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }
  try {
    motor2.writeThrottle(1.5);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }

  delay(1000);

  try {
    motor1.writeThrottle(1.5);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }
  try {
    motor2.writeThrottle(1.55);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }

  delay(1000);

  try {
    motor1.writeThrottle(1.5);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }
  try {
    motor2.writeThrottle(1.5);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }

  delay(1000);

  try {
    motor1.writeThrottle(1.55);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }
  try {
    motor2.writeThrottle(1.5);
  } catch (std::runtime_error& e) {
    Serial.println("Failed to set throttle: " + String(e.what()));
  }

  delay(1000);

};