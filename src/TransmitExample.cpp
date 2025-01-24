/**
 * @file TransmitExample.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief Example code to transmit a file using Ymodem protocol
 * @version 0.1
 * @date 2025-01-24
 *
 * This code is an example of how to use the Ymodem library to transmit a file
 * using the Ymodem protocol. The file is read from the SPIFFS filesystem and
 * sent through the Serial1 port.
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "YmodemCore.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>

#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024)        /*!< SPIFFS size in bytes */
#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000) /*!< Maximum file size */

Ymodem ymodem; /*!< Ymodem instance */

/**
 * @brief Initial system configuration.
 *
 * This function performs the following tasks:
 * - Initializes the Serial communication for debugging at 115200 baud.
 * - Waits until the Serial communication is ready.
 * - Mounts the SPIFFS filesystem and checks if it was mounted correctly.
 * - Configures the YModem activity LED pin.
 * - Initializes the UART (Serial1) with a speed of 115200 baud and configures the RX and TX pins.
 * - Prints a message indicating that the setup is complete and the system is ready to send files.
 */
void setup()
{

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS");
    return;
  }

  // Configure the Ymodem activity LED pin
  pinMode(YMODEM_LED_ACT, OUTPUT);
  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);

  // configure serial communication for Ymodem transfer
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // RX, TX pins
  if (!Serial1) {
    Serial.println("Error initialazing Serial port");
    return;
  }

  // setup finish message
  log_d("Setup completed, ready ro send data");
}

/**
 * @brief Main loop function that handles the transmission of a firmware file using Ymodem protocol.
 *
 * This function performs the following steps:
 * 1. Opens a file named "firmware-1.bin" from SPIFFS (SPI Flash File System).
 * 2. Checks if the file was successfully opened. If not, logs an error message and returns.
 * 3. Transmits the file to the receiver using the Ymodem protocol.
 * 4. Closes the file after transmission.
 * 5. Logs a success message if the transmission was successful, otherwise logs an error message with the error code.
 * 6. Introduces a short delay of 10 milliseconds before the next iteration of the loop.
 */
void loop()
{
  // Open file saved in SPIFFS to transmit
  File ffd = SPIFFS.open("/firmware-1.bin", FILE_READ);
  if (!ffd) {
    log_e("Error reading file form SPIFFS");
    return;
  }

  // Transmit the file to the receiver
  int result = ymodem.transmit((char*)"firmware-1.bin", ffd.size(), ffd);
  ffd.close();

  if (result == 0) {
    log_i("Send successfully done");
  }
  else {
    log_e("Error sending file. Error code %d\n", result);
  }
  delay(10);
}