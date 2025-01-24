/**
 * @file YmodemTransmit.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem Packet transmission functions
 * @version 0.1
 * @date 2025-01-24
 *
 * This file contains the functions to transmit files using the Ymodem protocol.
 * Format the packets and send them to the receiver or wait for the receiver response.
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "YmodemTransmit.h"

int waitForReceiverResponse()
{
  unsigned char receivedC;
  int           err = 0;

  do {
    Send_Byte(CRC16);
    LED_toggle();
  } while (Receive_Byte(&receivedC, NAK_TIMEOUT) < 0 && err++ < 45);

  if (err >= 45 || receivedC != CRC16) {
    send_CA();
    return -1;
  }

  return 0;
}

int sendInitialPacket(char* sendFileName, unsigned int sizeFile)
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  int     err;

  Ymodem_PrepareIntialPacket(packet_data, sendFileName, sizeFile);
  do {
    // Send Packet
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);

    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return -2; // timeout or wrong response
    }
    else if (err == 2)
      return 98; // abort
    LED_toggle();
  } while (err != 1);

  // After initial block the receiver sends 'C' after ACK
  if (Ymodem_WaitResponse(CRC16, 10) != 1) {
    send_CA();
    return -3;
  }

  return 0;
}

int sendFileBlocks(unsigned int sizeFile, fs::File& ffd)
{
  uint8_t  packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  uint16_t blkNumber = 0x01;
  int      err;
  uint32_t size = sizeFile;

  while (size) {
    // Prepare and send next packet
    Ymodem_PreparePacket(packet_data, blkNumber, size, ffd);
    do {
      // Send Packet
      uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD);

      // Wait for Ack
      err = Ymodem_WaitResponse(ACK, 10);
      if (err == 1) {
        blkNumber++;
        if (size > PACKET_1K_SIZE)
          size -= PACKET_1K_SIZE; // Next packet
        else
          size = 0; // Last packet sent
      }
      else if (err == 0 || err == 4) {
        send_CA();
        return -4; // timeout or wrong response
      }
      else if (err == 2)
        return -5; // abort
    } while (err != 1);
    LED_toggle();
  }

  return 0;
}

int sendEOT()
{
  int err;

  Send_Byte(EOT); // Send (EOT)
  do {
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 3) {   // NAK
      Send_Byte(EOT); // Send (EOT)
    }
    else if (err == 0 || err == 4) {
      send_CA();
      return -6; // timeout or wrong response
    }
    else if (err == 2)
      return -7; // abort
  } while (err != 1);

  return 0;
}

int sendLastPacket()
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  int     err;

  if (Ymodem_WaitResponse(CRC16, 10) != 1) {
    send_CA();
    return -8;
  }

  LED_toggle();
  Ymodem_PrepareLastPacket(packet_data);
  do {
    // Send Packet
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return -9; // timeout or wrong response
    }
    else if (err == 2)
      return -10; // abort
  } while (err != 1);

#if YMODEM_LED_ACT
  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
#endif

  return 0;
}