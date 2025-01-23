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

// int Ymodem::transmit(char* sendFileName, unsigned int sizeFile, fs::File& ffd)
// {
//   uint8_t       packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
//   uint16_t      blkNumber;
//   unsigned char receivedC;
//   int           i, err;
//   uint32_t      size = 0;

//   // Wait for response from receiver
//   err = 0;
//   do {
//     Send_Byte(CRC16);
//     LED_toggle();
//   } while (Receive_Byte(&receivedC, NAK_TIMEOUT) < 0 && err++ < 45);

//   if (err >= 45 || receivedC != CRC16) {
//     send_CA();
//     return -1;
//   }

//   // === Prepare first block and send it =======================================
//   /* When the receiving program receives this block and successfully
//    * opened the output file, it shall acknowledge this block with an ACK
//    * character and then proceed with a normal YMODEM file transfer
//    * beginning with a "C" or NAK tranmsitted by the receiver.
//    */
//   Ymodem_PrepareIntialPacket(packet_data, sendFileName, sizeFile);
//   do {
//     // Send Packet
//     uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);

//     // Wait for Ack
//     err = Ymodem_WaitResponse(ACK, 10);
//     if (err == 0 || err == 4) {
//       send_CA();
//       return -2; // timeout or wrong response
//     }
//     else if (err == 2)
//       return 98; // abort
//     LED_toggle();
//   } while (err != 1);

//   // After initial block the receiver sends 'C' after ACK
//   if (Ymodem_WaitResponse(CRC16, 10) != 1) {
//     send_CA();
//     return -3;
//   }

//   // === Send file blocks ======================================================
//   size      = sizeFile;
//   blkNumber = 0x01;

//   // Resend packet if NAK  for a count of 10 else end of communication
//   while (size) {
//     // Prepare and send next packet
//     Ymodem_PreparePacket(packet_data, blkNumber, size, ffd);
//     do {
//       // Send Packet
//       uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD);

//       // Wait for Ack
//       err = Ymodem_WaitResponse(ACK, 10);
//       if (err == 1) {
//         blkNumber++;
//         if (size > PACKET_1K_SIZE)
//           size -= PACKET_1K_SIZE; // Next packet
//         else
//           size = 0; // Last packet sent
//       }
//       else if (err == 0 || err == 4) {
//         send_CA();
//         return -4; // timeout or wrong response
//       }
//       else if (err == 2)
//         return -5; // abort
//     } while (err != 1);
//     LED_toggle();
//   }

//   // === Send EOT ==============================================================
//   Send_Byte(EOT); // Send (EOT)
//   // Wait for Ack
//   do {
//     // Wait for Ack
//     err = Ymodem_WaitResponse(ACK, 10);
//     if (err == 3) {   // NAK
//       Send_Byte(EOT); // Send (EOT)
//     }
//     else if (err == 0 || err == 4) {
//       send_CA();
//       return -6; // timeout or wrong response
//     }
//     else if (err == 2)
//       return -7; // abort
//   } while (err != 1);

//   // === Receiver requests next file, prepare and send last packet =============
//   if (Ymodem_WaitResponse(CRC16, 10) != 1) {
//     send_CA();
//     return -8;
//   }

//   LED_toggle();
//   Ymodem_PrepareLastPacket(packet_data);
//   do {
//     // Send Packet
//     uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);
//     // Wait for Ack
//     err = Ymodem_WaitResponse(ACK, 10);
//     if (err == 0 || err == 4) {
//       send_CA();
//       return -9; // timeout or wrong response
//     }
//     else if (err == 2)
//       return -10; // abort
//   } while (err != 1);

// #if YMODEM_LED_ACT
//   digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
// #endif
//   return 0; // file transmitted successfully
// }

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