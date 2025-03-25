/**
 * @file YmodemUtils.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem utility functions
 * @version 0.1
 * @date 2025-01-24
 *
 * This file contains utility functions used in the Ymodem protocol implementation.
 * These functions provide low-level functionality for sending and receiving data
 * over a serial connection using the Ymodem protocol.
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "YmodemUtils.h"

void IRAM_ATTR LED_toggle()
{
#if YMODEM_LED_ACT
  if (GPIO.out & (1 << YMODEM_LED_ACT)) {
    GPIO.out_w1tc = (1 << YMODEM_LED_ACT);
  }
  else {
    GPIO.out_w1ts = (1 << YMODEM_LED_ACT);
  }
#endif
}

unsigned short crc16(const unsigned char* buf, unsigned long count)
{
  unsigned short crc = 0;
  int            i;

  while (count--) {
    crc = crc ^ *buf++ << 8;

    for (i = 0; i < 8; i++) {
      if (crc & 0x8000)
        crc = crc << 1 ^ 0x1021;
      else
        crc = crc << 1;
    }
  }
  return crc;
}

int32_t Receive_Byte(unsigned char* c, uint32_t timeout)
{
  unsigned char ch;
  int           len = uart_read_bytes(EX_UART_NUM, &ch, 1, timeout / portTICK_RATE_MS);
  if (len <= 0)
    return -1;

  *c = ch;
  return 0;
}

void uart_consume()
{
  uint8_t ch[64];
  while (uart_read_bytes(EX_UART_NUM, ch, 64, 100 / portTICK_RATE_MS) > 0)
    ;
}

uint32_t Send_Byte(char c)
{
  int err = uart_write_bytes(EX_UART_NUM, &c, 1);
  if (err < 0) {
    return -1;
  }
  return 0;
}

void send_CA()
{
  Send_Byte(CA);
  Send_Byte(CA);
}

void send_ACK()
{
  Send_Byte(ACK);
}

void send_ACKCRC16()
{
  Send_Byte(ACK);
  Send_Byte(CRC16);
}

void send_NAK()
{
  Send_Byte(NAK);
}

void send_CRC16()
{
  Send_Byte(CRC16);
}

int32_t Receive_Packet(uint8_t* data, int* length, uint32_t timeout)
{
  int           count, packet_size, i;
  unsigned char ch;
  *length = 0;

  // receive 1st byte
  if (Receive_Byte(&ch, timeout) < 0) {
    return -1;
  }

  switch (ch) {
    case SOH:
      packet_size = PACKET_SIZE;
      break;
    case STX:
      packet_size = PACKET_1K_SIZE;
      break;
    case EOT:
      *length = 0;
      return 0;
    case CA:
      if (Receive_Byte(&ch, timeout) < 0) {
        return -2;
      }
      if (ch == CA) {
        *length = -1;
        return 0;
      }
      else
        return -1;
    case ABORT1:
    case ABORT2:
      return -2;
    default:
      vTaskDelay(100 / portTICK_RATE_MS);
      uart_consume();
      return -1;
  }

  *data         = (uint8_t)ch;
  uint8_t* dptr = data + 1;
  count         = packet_size + PACKET_OVERHEAD - 1;

  for (i = 0; i < count; i++) {
    if (Receive_Byte(&ch, timeout) < 0) {
      return -1;
    }
    *dptr++ = (uint8_t)ch;
    ;
  }

  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) {
    *length = -2;
    return 0;
  }
  if (crc16(&data[PACKET_HEADER], packet_size + PACKET_TRAILER) != 0) {
    *length = -2;
    return 0;
  }

  *length = packet_size;
  return 0;
}