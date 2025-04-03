/**
 * @file Update_LSM1X0A_Module.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief Example code to update the LSM1X0A module using Ymodem protocol
 * @version 0.1
 * @date 2025-04-02
 *
 * This code is an example of how to use the Ymodem library to update the LSM1X0A module
 * using the Ymodem protocol. The file to be transmitted is specified in the code.
 * The code initializes the Ymodem library, sets up the serial communication, and transmits the file.
 *
 * @note Make sure to replace the file path with the actual path of the file you want to transmit.
 *
 * @note This code assumes that the binary file is located in the SPIFFS filesystem.
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "YmodemCore.h"

void setup()
{
  Serial.begin(115200);

  const char* fileName = "/LSM100A_SDK_V104_240129.bin";
  // Transmit the file using Ymodem
  Ymodem ymodem;

  ymodem.resetExternalModule();

  YmodemPacketStatus err = ymodem.transmit(fileName);
  if (err == YMODEM_TRANSMIT_OK) {
    log_i("Success transmitting file: %s", fileName);
  }
  else {
    log_e("Error (%d): %s", err, ymodem.errorMessage(err));
  }
}

void loop()
{
}