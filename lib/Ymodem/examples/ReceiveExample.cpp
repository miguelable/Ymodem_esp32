/**
 * @file ReceiveExample.cpp
 * @brief  Example code to receive a file using Ymodem protocol
 * @version 0.1
 * @date 2025-01-24
 *
 * This code is an example of how to use the Ymodem library to receive a file
 * using the Ymodem protocol. The file is received through the Serial1 port and
 * saved to the SPIFFS filesystem.
 *
 */
#include "YmodemCore.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
#include <Update.h>

// Define CONFIG_SPIFFS_SIZE with an appropriate value
#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024)        /*!< SPIFFS size in bytes */
#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000) /*!< Maximum file size */
#define FIRMWARE_UPDATE                             /*!< Uncomment this line to enable firmware update */

Ymodem ymodem; /*!< Ymodem instance */

/**
 * @brief Setup function to initialize Serial, SPIFFS, and UART for YModem file reception.
 *
 * This function performs the following tasks:
 * - Initializes the Serial interface for debugging with a baud rate of 115200.
 * - Waits until the Serial interface is ready.
 * - Initializes the SPIFFS (SPI Flash File System) and checks for successful mounting.
 * - Configures the YModem activity LED pin and sets its initial state.
 * - Initializes the UART (Serial1) interface with a baud rate of 115200 and specified RX/TX pins.
 * - Logs a setup complete message indicating readiness to receive files.
 */
void setup()
{
  // Initialize Serial for debugging
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait until Serial is ready
  }

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error mounting SPIFFS");
    return;
  }

  // Configure the YModem activity LED pin
  pinMode(YMODEM_LED_ACT, OUTPUT);
  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);

  // Configure UART (Serial1)
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // RX, TX pins
  if (!Serial1) {
    log_e("Error initializing Serial1");
    return;
  }

  // Setup complete message
  log_i("Setup complete. Ready to receive files.");
}

/**
 * @brief Main loop function for receiving and updating firmware via YModem protocol.
 *
 * This function performs the following steps:
 * 1. Calculates the maximum file size that can be received based on the available space in SPIFFS.
 * 2. Opens a file in SPIFFS to store the received firmware.
 * 3. Initiates the YModem file reception and stores the received data in the opened file.
 * 4. If the file reception is successful, it proceeds to perform an OTA update with the received binary.
 * 5. If the OTA update is successful, it restarts the ESP32.
 * 6. Handles various error conditions such as file opening failures, transfer errors, and OTA update errors.
 *
 * @note The function uses the SPIFFS filesystem and the Update library for OTA updates.
 * @note The function assumes that the YModem library is properly initialized and configured.
 */
void loop()
{
  static int nfile = 1;
  char       fname[128];
  char       orig_name[256];
  uint32_t   max_fsize;
  int        rec_res = -1;

  // ==== File reception ====
  max_fsize = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  if (max_fsize > 16 * 1024) {
    if (max_fsize > MAX_FILE_SIZE) {
      max_fsize = MAX_FILE_SIZE;
    }

    sprintf(fname, "/firmware-%d.bin", nfile);
    File ffd = SPIFFS.open(fname, FILE_WRITE);
    if (ffd) {
      Serial.println("\r\nReceiving file, start YModem transfer on the host...\r\n");
      rec_res = ymodem.receive(ffd, max_fsize, orig_name);
      ffd.close();
      Serial.println("\r\n");

      if (rec_res > 0) {
        log_i("Transfer complete. Size=%d, Original name: \"%s\"", rec_res, fname);

#ifdef FIRMWARE_UPDATE
        // ==== Load the binary to the ESP32 ====
        File bin_file = SPIFFS.open(fname, FILE_READ);
        if (bin_file) {
          size_t bin_size = bin_file.size();
          if (Update.begin(bin_size)) {
            log_d("Starting OTA update...");
            size_t written = Update.writeStream(bin_file);
            if (written == bin_size)
              log_i("Write complete.");
            else
              log_w("Incomplete write. Only %d of %d bytes written", written, bin_size);

            if (Update.end()) {
              if (Update.isFinished()) {
                log_i("OTA update complete. Restarting...");
                while (1) {
                  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
                  vTaskDelay(100);
                }
              }
              else
                log_e("OTA update not complete.");
            }
            else
              log_e("Error in OTA update: %s", Update.errorString());
          }
          else
            log_e("Could not start OTA update.");
          bin_file.close();
        }
        else
          log_e("Error opening binary file \"%s\" for OTA.", fname);
#endif
      }
      else {
        log_e("Transfer error. Error code=%d", rec_res);
        SPIFFS.remove(fname);
      }
    }
    else
      log_e("Error opening file \"%s\" for receiving.", fname);

    delay(1000);
  }
  else {
    log_e("Filesystem full. Remaining space: %u bytes", max_fsize);
  }

  delay(10);
}