/**
 * @file YmodemPaquets.h
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem Packet preparation functions
 * @version 0.1
 * @date 2025-01-24
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef YMODEMPAQUETS_H
#define YMODEMPAQUETS_H

#include "YmodemUtils.h"

/**
 * @brief Prepares the initial packet for Ymodem transmission.
 *
 * This function initializes the first packet to be sent in a Ymodem transfer.
 * The initial packet contains metadata about the file being transferred, such
 * as the file name and its length.
 *
 * @param data Pointer to the buffer where the initial packet will be prepared.
 * @param fileName Pointer to a null-terminated string containing the name of the file.
 * @param length The length of the file in bytes.
 */
void Ymodem_PrepareIntialPacket(uint8_t* data, const char* fileName, uint32_t length);

/**
 * @brief Prepares the last packet for Ymodem transmission.
 *
 * This function is responsible for preparing the final packet to be sent
 * during a Ymodem file transfer. It ensures that the last packet is properly
 * formatted and ready for transmission.
 *
 * @param data Pointer to the buffer where the last packet data will be stored.
 */
void Ymodem_PrepareLastPacket(uint8_t* data);

/**
 * @brief Prepares a Ymodem packet with the given data.
 *
 * @param data Pointer to the buffer where the packet will be prepared.
 * @param packetNum Packet number to be included in the packet.
 * @param sizeBlk Size of the block to be included in the packet.
 * @param buffer Pointer to the data buffer to be included in the packet.
 */
void Ymodem_PreparePacket(uint8_t* data, uint8_t packetNum, uint32_t sizeBlk, const uint8_t* buffer);

/**
 * @brief Waits for a specific response character within a given timeout period.
 *
 * This function waits for a response character (ackchr) to be received within a specified
 * timeout period (tmo). It is typically used in communication protocols to ensure that
 * the expected response is received before proceeding.
 *
 * @param ackchr The expected response character to wait for.
 * @param timeout The timeout period in milliseconds to wait for the response.
 * @return YmodemPacketStatus Returns a status code indicating the result of the wait operation.
 */
YmodemPacketStatus Ymodem_WaitResponse(uint8_t ackchr, uint8_t timeout = WAIT_TIMEOUT);

#endif // YMODEMPAQUETS_H