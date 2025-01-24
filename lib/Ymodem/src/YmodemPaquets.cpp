/**
 * @file YmodemPaquets.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem Packet preparation functions
 * @version 0.1
 * @date 2025-01-24
 *
 * This file contains the functions to prepare the packets for Ymodem file transfer.
 * It formats the packets with the necessary information for the Ymodem protocol.
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "YmodemPaquets.h"

void Ymodem_PrepareIntialPacket(uint8_t* data, char* fileName, uint32_t length)
{
  uint16_t tempCRC;

  memset(data, 0, PACKET_SIZE + PACKET_HEADER);
  // Make first three packet
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;

  // add filename
  sprintf((char*)(data + PACKET_HEADER), "%s", fileName);

  // add file site
  sprintf((char*)(data + PACKET_HEADER + strlen((char*)(data + PACKET_HEADER)) + 1), "%d", length);
  data[PACKET_HEADER + strlen((char*)(data + PACKET_HEADER)) + 1 +
       strlen((char*)(data + PACKET_HEADER + strlen((char*)(data + PACKET_HEADER)) + 1))] = ' ';

  // add crc
  tempCRC                               = crc16(&data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER]     = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

void Ymodem_PrepareLastPacket(uint8_t* data)
{
  uint16_t tempCRC;

  memset(data, 0, PACKET_SIZE + PACKET_HEADER);
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;
  tempCRC = crc16(&data[PACKET_HEADER], PACKET_SIZE);
  // tempCRC = crc16_le(0, &data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER]     = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

void Ymodem_PreparePacket(uint8_t* data, uint8_t pktNo, uint32_t sizeBlk, fs::File& ffd)
{
  uint16_t i, size;
  uint16_t tempCRC;

  data[0] = STX;
  data[1] = (pktNo & 0x000000ff);
  data[2] = (~(pktNo & 0x000000ff));

  size = sizeBlk < PACKET_1K_SIZE ? sizeBlk : PACKET_1K_SIZE;
  // Read block from file
  if (size > 0) {
    size = ffd.read(data + PACKET_HEADER, size);
  }

  if (size < PACKET_1K_SIZE) {
    for (i = size + PACKET_HEADER; i < PACKET_1K_SIZE + PACKET_HEADER; i++) {
      data[i] = 0x00; // EOF (0x1A) or 0x00
    }
  }
  tempCRC = crc16(&data[PACKET_HEADER], PACKET_1K_SIZE);
  // tempCRC = crc16_le(0, &data[PACKET_HEADER], PACKET_1K_SIZE);
  data[PACKET_1K_SIZE + PACKET_HEADER]     = tempCRC >> 8;
  data[PACKET_1K_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

uint8_t Ymodem_WaitResponse(uint8_t ackchr, uint8_t tmo)
{
  unsigned char receivedC;
  uint32_t      errors = 0;

  do {
    if (Receive_Byte(&receivedC, NAK_TIMEOUT) == 0) {
      if (receivedC == ackchr) {
        return 1;
      }
      else if (receivedC == CA) {
        send_CA();
        return 2; // CA received, Sender abort
      }
      else if (receivedC == NAK) {
        return 3;
      }
      else {
        return 4;
      }
    }
    else {
      errors++;
    }
  } while (errors < tmo);
  return 0;
}