/**
 * @file YmodemCore.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem core functions
 * @version 0.1
 * @date 2025-01-24
 *
 * This file contains the core functions for the Ymodem protocol, including
 * both file transmission and reception. It provides a high-level interface
 * for sending and receiving files using the Ymodem protocol.
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "YmodemCore.h"

Ymodem::Ymodem()
{
  pinMode(ledPin, OUTPUT);
}

void Ymodem::setLedPin(int pin)
{
  ledPin = pin;
  // Set the LED pin as an output
  pinMode(ledPin, OUTPUT);
  log_i("LED pin set to %d", ledPin);
}

int Ymodem::getLedPin()
{
  return ledPin;
}

/**
 * @brief Finalizes the Ymodem session.
 *
 * This function is called to perform any necessary cleanup or finalization
 * tasks at the end of a Ymodem session. If YMODEM_LED_ACT is defined, it will
 * toggle the state of the LED connected to the specified pin.
 *
 * @note The LED state is toggled by XORing the current state with 1.
 */
void Ymodem::finalizeSession()
{
#if YMODEM_LED_ACT
  digitalWrite(ledPin, YMODEM_LED_ACT_ON ^ 1);
#endif
}

int Ymodem::receive(fs::File& ffd, unsigned int maxsize, char* getname)
{
  int          size         = 0;
  unsigned int session_done = 0, errors = 0;

  while (!session_done) {
    int result = handleFileSession(ffd, maxsize, getname, &session_done, &errors);
    if (result < 0) {
      size = result; // Código de error
      break;
    }
    size = result; // Tamaño del archivo recibido
  }

  finalizeSession();
  return size;
}

int Ymodem::transmit(const char* sendFileName)
{
  int        err;
  FileSystem fs;

  unsigned int sizeFile = fs.getFileSize(sendFileName);
  if (sizeFile == 0) {
    log_e("File not found: %s", sendFileName);
    return -5; // Filename packet error
  }

  // Correct the file name if it starts with '/'
  char* fileName = (char*)sendFileName;
  if (fileName[0] == '/') {
    fileName++;
  }

  // Wait for response from receiver
  err = waitForReceiverResponse();
  if (err != 0) {
    return err;
  }

  // Send initial packet
  err = sendInitialPacket(fileName, sizeFile);
  if (err != 0) {
    return err;
  }

  // Send file blocks
  err = sendFileBlocks(sendFileName, fs);
  if (err != 0) {
    return err;
  }

  // Send EOT
  err = sendEOT();
  if (err != 0) {
    return err;
  }

  // Send last packet
  err = sendLastPacket();
  if (err != 0) {
    return err;
  }

  return 0; // file transmitted successfully
}