/**
 * @file YmodemUtils.h
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief    Ymodem utility functions
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
#ifndef YMODEMUTILS_H
#define YMODEMUTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "filesystem.h"
#include "rom/crc.h"
#include <driver/uart.h>

#include "YmodemDef.h"

enum ByteOperationStatus : int8_t
{
  BYTE_OK    = 0,  // Successful byte operation
  BYTE_ERROR = -1, // Error in byte operation
};

enum PacketLengthStatus : int8_t
{
  PACKET_EOT       = 0,  // End of transmission
  PACKET_ABORT   = -1, // Abort by sender (CAN character)
  PACKET_SEQ_INVALID = -2, // Paquet sequence error (invalid sequence number)
  PACKET_CRC_INVALID = -3  // Packet CRC error (invalid checksum)
};

enum ReceivePacketStatus : int8_t
{
  PACKET_OK              = 0,  // Packet received successfully
  PACKET_INVALID_CA      = -1, // Error reading byte (invalid CA sequence)
  PACKET_INVALID_HEADER  = -2, // Error reading byte (invalid header)
  PACKET_TIMEOUT         = -3, // Error reading byte (timeout)
  PACKET_SECOND_TIMEOUT  = -4, // Error reading second confirmation byte (timeout)
  PACKET_ABORTED         = -5, // Transmission aborted by sender
  PACKET_SEQ_ERROR       = -6, // Error in the packet sequence number
  PACKET_CRC_ERROR       = -7, // Error in the packet CRC
  PACKET_BUFFER_OVERFLOW = -8  // Buffer overflow
};

/**
 * @brief Toggles the state of an LED.
 *
 * This function is marked with the IRAM_ATTR attribute, indicating that it should be placed in
 * the IRAM (Instruction RAM) for faster execution. This is typically used for functions that
 * need to be executed quickly, such as interrupt service routines.
 *
 * Note: Ensure that the LED is properly initialized before calling this function.
 */
void IRAM_ATTR LED_toggle();

/**
 * @brief Computes the CRC-16 checksum for a given buffer using the CCITT-FALSE polynomial.
 *
 * This function calculates the 16-bit cyclic redundancy check (CRC) for the input buffer
 * using the polynomial 0x1021 (CCITT-FALSE). The CRC is initialized to 0 and processed
 * byte-by-byte. Each byte is XORed with the upper byte of the CRC, and the CRC is updated
 * by shifting and optionally XORing with the polynomial based on the most significant bit.
 *
 * @param buf Pointer to the input buffer containing the data to compute the CRC for.
 * @param count The number of bytes in the input buffer.
 * @return The computed 16-bit CRC value.
 */
unsigned short crc16(const unsigned char* buf, size_t count);

/**
 * @brief Receives a byte from a communication interface.
 *
 * This function attempts to receive a single byte from a communication interface
 * within a specified timeout period.
 *
 * @param[out] c Pointer to an unsigned char where the received byte will be stored.
 * @param[in] timeout The maximum time to wait for a byte to be received, in milliseconds.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
ByteOperationStatus Receive_Byte(unsigned char* c, uint32_t timeout);

/**
 * @brief Sends an End Of Transmission (EOT) signal.
 * 
 * This function is used to indicate the end of data transmission
 * in communication protocols such as YMODEM. It typically signals
 * the receiver that no more data will be sent.
 */
void send_EOT();

/**
 * @brief Sends the CA (Cancel) signal.
 *
 * This function is used to send the CA (Cancel) signal, which is typically
 * used to abort an ongoing operation or communication.
 */
void send_CA();

/**
 * @brief Sends an acknowledgment (ACK) signal.
 *
 * This function is used to send an ACK signal to indicate successful
 * receipt of data or successful completion of a process.
 */
void send_ACK();

/**
 * @brief Sends an acknowledgment with CRC16.
 *
 * This function sends an acknowledgment (ACK) signal with a CRC16 checksum.
 * It is typically used in communication protocols to confirm the successful
 * receipt of data.
 */
void send_ACKCRC16();

/**
 * @brief Sends a Negative Acknowledgement (NAK) signal.
 *
 * This function is used to send a NAK signal, typically in communication
 * protocols, to indicate that the received data is not valid or an error
 * has occurred.
 */
void send_NAK();

/**
 * @brief Sends the CRC16 checksum.
 *
 * This function calculates and sends the CRC16 checksum for data integrity verification.
 * It is typically used in communication protocols to ensure that the transmitted data
 * has not been corrupted.
 */
void send_CRC16();

/**
 * @brief Receives a packet of data.
 *
 * This function attempts to receive a packet of data within a specified timeout period.
 *
 * @param data Pointer to the buffer where the received data will be stored.
 * @param length Pointer to an integer where the length of the received data will be stored.
 * @param timeout The maximum time to wait for a packet, in milliseconds.
 * @return int32_t Returns the status of the packet reception. A non-negative value indicates success,
 *                 while a negative value indicates an error.
 */
ReceivePacketStatus ReceiveAndValidatePacket(uint8_t* data, int* length, uint32_t timeout);

#endif // YMODEMUTILS_H