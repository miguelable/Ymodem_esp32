
#include "YmodemCore.h"

Ymodem ymodem;

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  ymodem.setLedPin(2);

  FileSystem file;

  // Save the file system object
  const char* fileName = "/test.txt";
  for (int i = 0; i < 100; i++) {
    char data[50];
    snprintf(data, sizeof(data), "Hello, World! %d\n", i);
    file.writeToFile(fileName, data, strlen(data));
  }
  file.~FileSystem();

  // Transmit the file using Ymodem
  if (!ymodem.transmit(fileName)) {
    log_i("File transmitted successfully");
  }
  else {
    log_e("Error transmitting file");
  }
}

void loop()
{
}