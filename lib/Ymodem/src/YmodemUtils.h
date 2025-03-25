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
 * @brief Computes the CRC-16 checksum for a given buffer.
 *
 * This function calculates the CRC-16 checksum for the provided buffer using
 * a standard polynomial. The CRC-16 checksum is commonly used for error-checking
 * in data transmission and storage.
 *
 * @param buf Pointer to the buffer containing the data to be checksummed.
 * @param count The number of bytes in the buffer.
 * @return The computed CRC-16 checksum as an unsigned short.
 */
unsigned short crc16(const unsigned char* buf, unsigned long count);

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
int32_t Receive_Byte(unsigned char* c, uint32_t timeout);

/**
 * @brief Consumes data from the UART buffer.
 *
 * This function reads and discards data from the UART buffer. It is typically used
 * to clear the buffer of any unwanted or residual data before starting a new communication
 * session or operation.
 */
void uart_consume();

/**
 * @brief Sends a single byte of data.
 *
 * This function transmits a single character (byte) over a communication interface.
 *
 * @param c The character to be sent.
 * @return uint32_t Returns a status code indicating the result of the operation.
 */
uint32_t Send_Byte(char c);

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
int32_t Receive_Packet(uint8_t* data, int* length, uint32_t timeout);

int readFirmware(uint8_t* buffer, size_t offset, size_t length);

#endif // YMODEMUTILS_H