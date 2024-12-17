#include "ymodem.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>

// Create an instance of the Ymodem class
Ymodem ymodem(&Serial1);

// Create a file to send in SPIFFS
void createFile() {
  // Open a file in write mode
  File file = SPIFFS.open("/file_to_send.bin", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to create file");
    return;
  }

  // Write some data to the file
  for (int i = 0; i < 150; i++) {
    file.write(i);
  }

  // Close the file
  file.close();
}

bool sendFile() {
  File file = SPIFFS.open("/file_to_send.bin", FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  }

  // Get the file size
  unsigned int fileSize = file.size();

  // Convert fs::File to FILE*
  FILE *filePtr = fopen("/spiffs/file_to_send.bin", "rb");
  if (!filePtr) {
    Serial.println("Failed to open file for reading");
    return false;
  } else {
    Serial.println("File opened for reading");
    // print file
    // char c;
    // Serial.println("File content:");
    // while (fread(&c, 1, 1, filePtr) == 1) {
    //   Serial.printf("%02X ", c);
    // }
    // Serial.println();
  }

  // Call the Ymodem_Transmit function
  int result = ymodem.transmit("file_to_send.bin", fileSize, filePtr);
  // int result = 0;

  // Close the FILE* pointer
  fclose(filePtr);

  // Close the file
  file.close();

  // Check the result
  if (result == 0) {
    Serial.println("File sent successfully");
    return true;
  } else {
    Serial.printf("Failed to send file. Error code: %d\n", result);
    return false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("YModem Transmit Example");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Create a file to send
  createFile();
  ymodem.setLedPin(2);
}

void loop() {
  // put your main code here, to run repeatedly:
  static bool sent = false;
  if (!sent) {
    // Send the file
    if (sendFile())
      sent = true;
  }
  // Add a delay to avoid flooding the serial output
  delay(1000);
}