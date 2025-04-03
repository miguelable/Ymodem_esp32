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

// CRC Precomputed table for the CCITT-FALSE polynomial (0x1021)
static const uint16_t crc16_table[256] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210,
  0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
  0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6,
  0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC,
  0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
  0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
  0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB,
  0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
  0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
  0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07,
  0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
  0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};

void IRAM_ATTR LED_toggle()
{
#if YMODEM_LED_ACT
  if (GPIO.out & (1 << YMODEM_LED_ACT))
    GPIO.out_w1tc = (1 << YMODEM_LED_ACT);
  else
    GPIO.out_w1ts = (1 << YMODEM_LED_ACT);
#endif
}

/* Old crc16 calculation function (slower)
unsigned short crc16(const unsigned char* buf, size_t count)
{
  unsigned short crc = 0;
  while (count--) {
    crc = crc ^ *buf++ << 8;
    for (int i = 0; i < 8; i++) {
      if (crc & 0x8000)
        crc = crc << 1 ^ 0x1021;
      else
        crc = crc << 1;
    }
  }
  return crc;
}
*/

unsigned short crc16(const unsigned char* buf, size_t count)
{
  unsigned short crc = 0;
  while (count--) {
    uint8_t tableIndex = (crc >> 8) ^ *buf++;
    crc                = (crc << 8) ^ crc16_table[tableIndex];
  }
  return crc;
}

ByteOperationStatus Receive_Byte(unsigned char* c, uint32_t timeout)
{
  unsigned char ch;
  int           err = uart_read_bytes(EX_UART_NUM, &ch, 1, timeout / portTICK_RATE_MS);
  if (err <= 0)
    return BYTE_ERROR;
  *c = ch;
  return BYTE_OK;
}

void uart_consume()
{
  uint8_t ch[64];
  while (uart_read_bytes(EX_UART_NUM, ch, 64, 100 / portTICK_RATE_MS) > 0)
    ;
}

ByteOperationStatus Send_Byte(char c)
{
  int err = uart_write_bytes(EX_UART_NUM, &c, 1);
  if (err < 0)
    return BYTE_ERROR;
  return BYTE_OK;
}

void send_EOT()
{
  Send_Byte(EOT);
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

void handleSOH(int* packet_size)
{
  *packet_size = PACKET_SIZE;
}

void handleSTX(int* packet_size)
{
  *packet_size = PACKET_1K_SIZE;
}

/**
 * @brief Handles the End Of Transmission (EOT) signal in the Ymodem protocol.
 *
 * This function processes the EOT signal received during a Ymodem file transfer,
 * sets the provided length pointer to the EOT packet identifier, sends an
 * acknowledgment (ACK) to the sender, and returns the status of the operation.
 *
 * @param length Pointer to an integer where the EOT packet identifier will be stored.
 *               This is set to the value of PACKET_EOT.
 * @return YmodemPacketStatus Returns YMODEM_RECEIVED_OK to indicate successful handling of the EOT signal.
 */
YmodemPacketStatus handleEOT(int* length)
{
  *length = PACKET_EOT;
  send_ACK();
  return YMODEM_RECEIVED_OK;
}

/**
 * @brief Handles the reception of a CA (Cancel) character during a YMODEM transfer.
 *
 * This function checks if the received byte matches the CA character. If it does,
 * it sends an acknowledgment (ACK) and returns a status indicating successful handling.
 * If the byte is not received within the specified timeout, or if the received byte
 * does not match the CA character, appropriate error statuses are returned.
 *
 * @param ch Pointer to a variable where the received byte will be stored.
 * @param timeout The maximum time (in milliseconds) to wait for a byte to be received.
 * @param length Pointer to an integer where the length of the packet will be stored.
 * @return YmodemPacketStatus
 *         - YMODEM_TIMEOUT: If no byte is received within the timeout period.
 *         - YMODEM_RECEIVED_OK: If the received byte matches the CA character and ACK is sent.
 *         - YMODEM_INVALID_HEADER: If the received byte does not match the CA character.
 */
YmodemPacketStatus handleCA(unsigned char* ch, uint32_t timeout, int* length)
{
  if (Receive_Byte(ch, timeout) < 0) {
    return YMODEM_TIMEOUT;
  }
  if (*ch == CA) {
    *length = PACKET_ABORT;
    send_ACK();
    return YMODEM_RECEIVED_OK;
  }
  return YMODEM_INVALID_HEADER;
}

/**
 * @brief Handles the abort signal during a YMODEM file transfer.
 *
 * This function is called when an abort signal is detected, indicating
 * that the file transfer process should be terminated. It returns a
 * status indicating that the packet transfer has been aborted.
 *
 * @return YMODEM_ABORTED_BY_SENDER Status indicating the transfer was aborted.
 */
YmodemPacketStatus handleAbort()
{
  return YMODEM_ABORTED_BY_SENDER;
}

/**
 * @brief Handles an invalid header in a received packet.
 *
 * This function introduces a delay, consumes any remaining data in the UART buffer,
 * and returns a status indicating that the packet header is invalid.
 *
 * @return YMODEM_INVALID_HEADER to indicate an invalid packet header.
 */
YmodemPacketStatus handleInvalidHeader()
{
  vTaskDelay(100 / portTICK_RATE_MS);
  uart_consume();
  return YMODEM_INVALID_HEADER;
}

/**
 * @brief Receives the initial byte of a packet with a specified timeout.
 *
 * This function attempts to receive a single byte and determines the status
 * of the operation based on whether the byte was successfully received within
 * the given timeout period.
 *
 * @param[out] ch Pointer to a variable where the received byte will be stored.
 * @param[in] timeout The maximum time (in milliseconds) to wait for the byte.
 * @return YMODEM_RECEIVED_OK if the byte was successfully received,
 *         YMODEM_TIMEOUT if the operation timed out.
 */
YmodemPacketStatus ReceiveInitialByte(unsigned char* ch, uint32_t timeout)
{
  if (Receive_Byte(ch, timeout) < 0) {
    return YMODEM_TIMEOUT;
  }
  return YMODEM_RECEIVED_OK;
}

/**
 * @brief Handles the header of a received packet and determines the appropriate action.
 *
 * @param ch The header byte of the packet to process.
 * @param packet_size Pointer to an integer where the packet size will be stored if applicable.
 * @param length Pointer to an integer where the length of the packet will be stored if applicable.
 * @param timeout The timeout value for operations that require waiting.
 * @param data Pointer to a byte where the processed header byte will be stored.
 * @return YmodemPacketStatus The status of the packet processing, indicating success or the type of error.
 *
 * This function processes the header byte of a packet and determines the appropriate action
 * based on its value. It supports the following header types:
 * - SOH: Start of Header, processes a small packet.
 * - STX: Start of Header, processes a larger packet.
 * - EOT: End of Transmission, signals the end of data transfer.
 * - CA: Cancel, cancels the transmission.
 * - ABORT1/ABORT2: Aborts the operation.
 * - Default: Handles invalid or unrecognized headers.
 */
YmodemPacketStatus HandlePacketHeader(unsigned char ch, int* packet_size, int* length, uint32_t timeout, uint8_t* data)
{
  switch (ch) {
    case SOH:
      handleSOH(packet_size);
      break;
    case STX:
      handleSTX(packet_size);
      break;
    case EOT:
      return handleEOT(length);
    case CA:
      return handleCA(&ch, timeout, length);
    case ABORT1:
    case ABORT2:
      return handleAbort();
    default:
      return handleInvalidHeader();
  }

  *data = (uint8_t)ch;
  return YMODEM_RECEIVED_OK;
}

/**
 * @brief Reads packet data from a source with a specified timeout.
 *
 * This function reads a packet of data of the specified size, including
 * the packet overhead, from a source. It ensures that the data does not
 * exceed the maximum buffer size and handles timeout or buffer overflow
 * conditions appropriately.
 *
 * @param data Pointer to the buffer where the received packet data will be stored.
 *             The first byte of the buffer is skipped, and data is written starting
 *             from the second byte.
 * @param packet_size The size of the packet to be read (excluding overhead).
 * @param timeout The timeout duration (in milliseconds) for receiving each byte.
 *
 * @return YMODEM_RECEIVED_OK if the packet is successfully read.
 *         YMODEM_TIMEOUT if a timeout occurs while receiving a byte.
 *         YMODEM_BUFFER_OVERFLOW if the buffer size is exceeded.
 */
YmodemPacketStatus ReadPacketData(uint8_t* data, int packet_size, uint32_t timeout)
{
  uint8_t*      dptr = data + 1;
  unsigned char ch;

  for (int i = 0; i < (packet_size + PACKET_OVERHEAD - 1); i++) {
    if (Receive_Byte(&ch, timeout) < 0) {
      return YMODEM_TIMEOUT;
    }
    if (dptr - data >= MAX_BUFFER_SIZE) {
      return YMODEM_BUFFER_OVERFLOW;
    }
    *dptr++ = (uint8_t)ch;
  }

  return YMODEM_RECEIVED_OK;
}

/**
 * @brief Validates a received packet by checking its sequence number and CRC.
 *
 * @param data Pointer to the packet data buffer.
 * @param packet_size Size of the packet data (excluding header and trailer).
 * @param length Pointer to an integer where the function will store the result:
 *               - If the sequence number is invalid, it will store YMODEM_SEQ_ERROR.
 *               - If the CRC check fails, it will store YMODEM_CRC_ERROR.
 *               - Otherwise, it will store the packet size.
 * @return YMODEM_RECEIVED_OK if the packet is valid or if an error is detected.
 *
 * @note The function always returns YMODEM_RECEIVED_OK, but the actual status is indicated
 *         through the `length` parameter.
 */
YmodemPacketStatus ValidatePacket(uint8_t* data, int packet_size, int* length)
{
  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) {
    *length = PACKET_SEQ_INVALID;
    return YMODEM_RECEIVED_OK;
  }

  if (crc16(&data[PACKET_HEADER], packet_size + PACKET_TRAILER) != 0) {
    *length = PACKET_CRC_INVALID;
    return YMODEM_RECEIVED_OK;
  }

  *length = packet_size;
  return YMODEM_RECEIVED_OK;
}

YmodemPacketStatus ReceiveAndValidatePacket(uint8_t* data, int* length, uint32_t timeout)
{
  int           packet_size;
  unsigned char ch;

  *length = PACKET_EOT;

  // Receive the initial byte
  YmodemPacketStatus status = ReceiveInitialByte(&ch, timeout);
  if (status != YMODEM_RECEIVED_OK) {
    return status;
  }

  // Handle the packet header (SOH, STX, EOT, CA, ABORT)
  status = HandlePacketHeader(ch, &packet_size, length, timeout, data);
  if (status != YMODEM_RECEIVED_OK) {
    return status;
  }

  // Read the packet data
  status = ReadPacketData(data, packet_size, timeout);
  if (status != YMODEM_RECEIVED_OK) {
    return status;
  }

  // Validate the packet sequence and CRC
  return ValidatePacket(data, packet_size, length);
}