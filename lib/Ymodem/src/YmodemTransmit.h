/**
 * @file YmodemTransmit.h
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
#ifndef YMODEMTRANSMIT_H
#define YMODEMTRANSMIT_H

#include "YmodemPaquets.h"

/**
 * @brief Waits for a response from the receiver.
 *
 * This function blocks the execution until a response is received from the receiver.
 * It is typically used in communication protocols where the transmitter needs to wait
 * for an acknowledgment or other response from the receiver before proceeding.
 *
 * @return int Returns a status code indicating the result of waiting for the receiver's response.
 *             The specific values and their meanings should be defined in the implementation.
 */
int waitForReceiverResponse();

/**
 * @brief Sends the initial packet for a file transfer using the Ymodem protocol.
 *
 * This function prepares and sends the initial packet containing the file name
 * and size to initiate the Ymodem file transfer process.
 *
 * @param sendFileName The name of the file to be sent.
 * @param sizeFile The size of the file to be sent, in bytes.
 * @return int Returns 0 on success, or a negative error code on failure.
 */
int sendInitialPacket(char* sendFileName, unsigned int sizeFile);

/**
 * @brief Sends file blocks over a communication channel.
 *
 * This function is responsible for transmitting the contents of a file in blocks.
 *
 * @param sizeFile The size of the file to be sent, in bytes.
 * @param ffd A reference to the file object to be transmitted.
 * @return int Returns 0 on success, or a negative error code on failure.
 */
int sendFileBlocks(unsigned int sizeFile, fs::File& ffd);

/**
 * @brief Sends the End Of Transmission (EOT) signal.
 *
 * This function is used to signal the end of a transmission process.
 * It typically sends an EOT character to indicate that the transmission
 * is complete.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 */
int sendEOT();

/**
 * @brief Sends the last packet in the Ymodem transmission.
 *
 * This function is responsible for transmitting the final packet
 * in the Ymodem protocol. It ensures that the transmission is
 * properly terminated.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 */
int sendLastPacket();

#endif // YMODEMTRANSMIT_H