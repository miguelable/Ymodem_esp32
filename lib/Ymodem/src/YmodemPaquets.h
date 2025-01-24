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
void    Ymodem_PrepareIntialPacket(uint8_t* data, char* fileName, uint32_t length);

/**
 * @brief Prepares the last packet for Ymodem transmission.
 *
 * This function is responsible for preparing the final packet to be sent
 * during a Ymodem file transfer. It ensures that the last packet is properly
 * formatted and ready for transmission.
 *
 * @param data Pointer to the buffer where the last packet data will be stored.
 */
void    Ymodem_PrepareLastPacket(uint8_t* data);

/**
 * @brief Prepares a Ymodem packet with the given data.
 * 
 * @param data Pointer to the buffer where the packet will be prepared.
 * @param pktNo Packet number to be included in the packet.
 * @param sizeBlk Size of the block to be included in the packet.
 * @param ffd Reference to the file from which data will be read.
 */
void    Ymodem_PreparePacket(uint8_t* data, uint8_t pktNo, uint32_t sizeBlk, fs::File& ffd);

/**
 * @brief Waits for a specific response character within a given timeout period.
 *
 * This function waits for a response character (ackchr) to be received within a specified
 * timeout period (tmo). It is typically used in communication protocols to ensure that
 * the expected response is received before proceeding.
 *
 * @param ackchr The expected response character to wait for.
 * @param tmo The timeout period in milliseconds to wait for the response.
 * @return uint8_t Returns 1 if the expected response character is received within the timeout period,
 *         otherwise returns 0.
 */
uint8_t Ymodem_WaitResponse(uint8_t ackchr, uint8_t tmo);

#endif // YMODEMPAQUETS_H