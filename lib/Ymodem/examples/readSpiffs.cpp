/**
 * @file readSpiffs.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Example code to read files from SPIFFS
 * @version 0.1
 * @date 2025-01-24
 *
 * This code is an example of how to read files from the SPIFFS filesystem
 * using the ESP32 SPIFFS library. It lists the files in SPIFFS, reads and
 * displays the content of each file, and optionally deletes the files after
 * reading.
 *
 * This file is used to demonstrate the SPIFFS library functionality and how
 * to interact with the filesystem on the ESP32. It can be used to check the
 * contents transferred using the Ymodem protocol.
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <Arduino.h>
#include <SPIFFS.h>

#define MAX_DEBUG    /*!< Comment this line to disable file content display */
#define DELETE_FILES /*!< Comment this line to keep the files after reading */

/**
 * @brief Setup function for initializing Serial communication and SPIFFS.
 *
 * This function performs the following tasks:
 * 1. Initializes Serial communication at a baud rate of 115200 for debugging purposes.
 * 2. Waits until the Serial is ready.
 * 3. Initializes the SPIFFS (SPI Flash File System).
 * 4. Lists all files present in the SPIFFS.
 * 5. Optionally reads and displays the content of each file if MAX_DEBUG is defined.
 * 6. Optionally deletes all files in the SPIFFS if DELETE_FILES is defined.
 * 7. Lists all files present in the SPIFFS after deletion if DELETE_FILES is defined.
 *
 * @note Ensure that the SPIFFS library is included and properly configured in the project.
 * @note Define MAX_DEBUG to enable reading and displaying file content.
 * @note Define DELETE_FILES to enable deletion of all files in the SPIFFS.
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

  // List files in SPIFFS
  Serial.println("Files in SPIFFS:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    log_v("Name: %s", file.name());
// Read and display the entire content of the file
#ifdef MAX_DEBUG
    Serial.print("Content: ");
    while (file.available()) {
      char buffer[101];                                // Buffer to store the read characters
      int  bytesRead    = file.readBytes(buffer, 100); // Read up to 100 characters
      buffer[bytesRead] = '\0';                        // Ensure the buffer is null-terminated
      Serial.print(buffer);
    }
    Serial.println(); // New line after displaying the file content
#endif
    file = root.openNextFile();
  }

  // Delete all files in SPIFFS
#ifdef DELETE_FILES
  root = SPIFFS.open("/");
  file = root.openNextFile();
  while (file) {
    const char* fileName = file.name();
    if (fileName && strlen(fileName) > 0) {
      log_w("Deleting file: %s", fileName);
      if (SPIFFS.remove(fileName)) {
        log_i("File deleted successfully");
      }
      else {
        log_e("Error deleting file");
      }
    }
    else {
      log_w("Invalid file name");
    }
    file = root.openNextFile();
  }

  // List files in SPIFFS after deletion
  Serial.println("Files in SPIFFS after deletion:");
  root = SPIFFS.open("/");
  file = root.openNextFile();
  while (file) {
    Serial.print("Name: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
#endif
}

/**
 * @brief Main loop function.
 *
 * This function is intentionally left empty as no code is needed
 * to be executed repeatedly for this operation.
 */
void loop()
{
  // No code needed in the loop for this operation
}