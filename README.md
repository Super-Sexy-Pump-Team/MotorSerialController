# ESP32 Castle Creations ESC Controller

This project is for developing an ESP32-S3 library to commmunicate with the Castle ESC to control the motors. It communicates with the ESC using UART and reads various parameters such as voltage, current, throttle, and RPM. It also allows setting the throttle value. The library is located in the `lib` folder and can be copied to other projects. This was developed specifically for the ESP32-S3-DevKitC-1-N32R8V board but can be used with other boards by changing the `platformio.ini` file.

## Features

- Read voltage, current, throttle, and RPM from the ESC
- Set throttle value
- Serial communication for debugging and control


## Getting Started

### Prerequisites

- PlatformIO
- Visual Studio Code
- ESP32-S3 Board (S3-DevKitC-1-N32R8V recommended)

### Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/TobySagi/TobyMotorPIDLoop
    ```
2. Open the project in Visual Studio Code.
3. Install PlatformIO extension for Visual Studio Code.

### Building and Uploading

1. Connect your ESP32-S3 DevKitC-1 to your computer.
2. Open a terminal in Visual Studio Code.
3. Upload the firmware using the platform.io gui or by using:
    ```sh
    pio run --target upload
    ```

### Usage

1. Open the serial monitor.
2. The ESP32-S3 will initialize and start reading data from the ESC and display it to the terminal.
3. You can send throttle values (between 1.0 and 2.0 ms) via the serial monitor to control the ESC by typing the value in the terminal and hitting the enter key.

By default, the program assumes you will be using the platform.io serial monitor built into VS Code. If you are using an external serial monitor that supports ANSI escape codes, you can type `e` in the terminal to enable the use of escape codes for better data logging. Pressing `d` will once again disble the escape codes.


## Library Functions

### `float readVoltage(uint8_t deviceId)`

Reads the motor voltage from the Castle ESC.

- **Parameters:**
  - `deviceId` (uint8_t): The ID of the device to read from.
- **Returns:**
  - `float`: The voltage value (0-20V). Returns -1.0 if an error occurs.

### `float readThrottle(uint8_t deviceId)`

Reads the throttle pulse time from the Castle ESC

- **Parameters:**
  - `deviceId` (uint8_t): The ID of the device to read from.
- **Returns:**
  - `float`: The throttle pulse time (1-2ms). Returns -1.0 if an error occurs.

### `float readCurrent(uint8_t deviceId)`

Reads the motor current from the Castle ESC

- **Parameters:**
  - `deviceId` (uint8_t): The ID of the device to read from.
- **Returns:**
  - `float`: The current value (0-50A). Returns -1.0 if an error occurs.

### `float readRPM(uint8_t deviceId)`

Read the motor RPM from the Castle ESC

- **Parameters:**
  - `deviceId` (uint8_t): The ID of the device to read from.
- **Returns:**
  - `float`: The motor RPM (0-20416). Returns -1.0 if an error occurs.

### `bool writeThrottle(uint8_t deviceId, float throttleMs)`

Set the throttle pulse time of the motor

- **Parameters:**
  - `deviceId` (uint8_t): The ID of the device to read from.
  - `throttleMs` (float): The throttle pulse time in milliseconds.
- **Returns:**
  - `bool`: Returns `true` on successful write.

## To-Do List

- [ ] Restructure the library into an object-oriented design (create motor class)
- [ ] Integrate with web gui for remote control and monitoring
