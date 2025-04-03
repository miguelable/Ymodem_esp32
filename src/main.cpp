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