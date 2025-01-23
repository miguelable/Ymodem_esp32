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

void Ymodem::finalizeSession()
{
#if YMODEM_LED_ACT
  digitalWrite(ledPin, YMODEM_LED_ACT_ON ^ 1);
#endif
}

/**
 * @brief Receives a file using the Ymodem protocol.
 *
 * This function handles the reception of a file over a Ymodem connection.
 * It processes packets, handles errors, and writes the received data to the specified file.
 *
 * @param ffd Reference to the file object where the received data will be written.
 * @param maxsize Maximum allowed size of the file to be received.
 * @param getname Pointer to a character array where the received file name will be stored.
 * @return int The size of the received file in bytes, or a negative error code:
 *         - -1: Abort by sender
 *         - -2: Too many errors
 *         - -3: Packet sequence error
 *         - -4: Invalid file size
 *         - -5: Filename packet error
 *         - -6: File write error
 *         - -7: User abort
 *         - -8: Timeout
 *         - -9: File size exceeds maxsize
 */
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

int Ymodem::transmit(char* sendFileName, unsigned int sizeFile, fs::File& ffd)
{
  int err;

  // Wait for response from receiver
  err = waitForReceiverResponse();
  if (err != 0) {
    return err;
  }

  // Send initial packet
  err = sendInitialPacket(sendFileName, sizeFile);
  if (err != 0) {
    return err;
  }

  // Send file blocks
  err = sendFileBlocks(sizeFile, ffd);
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