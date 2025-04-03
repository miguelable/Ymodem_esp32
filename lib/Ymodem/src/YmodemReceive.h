/**
 * @file YmodemReceive.h
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem Packet reception functions
 * @version 0.1
 * @date 2025-01-24
 *
 * This file contains the functions to receive files using the Ymodem protocol.
 * It processes the incoming packets, extracts the file information, and writes
 * the data to the filesystem.
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef YMODEMRECEIVE_H
#define YMODEMRECEIVE_H

#include "YmodemUtils.h"

/**
 * @brief Processes a data packet received via Ymodem protocol.
 *
 * This function handles the data packet, writing its contents to the specified file,
 * and updating the error count if any issues are encountered.
 *
 * @param packet_data Pointer to the data packet to be processed.
 * @param packet_length Length of the data packet.
 * @param ffd Reference to the file object where the data will be written.
 * @param file_size Total size of the file being received.
 * @param errors Pointer to an unsigned int where the error count will be updated.
 * @return ReceivePacketStatus Status of the packet processing.
 *         - PACKET_OK: Packet processed successfully.
 *         - PACKET_ERROR_WRITING: Error writing to the file in the filesystem.
 */
ReceivePacketStatus processDataPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int file_size);

/**
 * @brief Handles the End Of File (EOF) packet in the Ymodem protocol.
 *
 * This function processes the EOF packet received during a Ymodem file transfer.
 * It updates the status of the file transfer and tracks any errors encountered.
 *
 * @param file_done Pointer to an unsigned int that indicates whether the file transfer is complete.
 *                  A non-zero value indicates completion.
 * @param errors Pointer to an unsigned int that tracks the number of errors encountered during the transfer.
 */
void handleEOFPacket(unsigned int* file_done, unsigned int* errors);

/**
 * @brief Extracts file information from a Ymodem packet.
 *
 * This function parses the given packet data to extract the file name and size.
 *
 * @param packet_data Pointer to the packet data containing the file information.
 * @param getname Pointer to a character array where the extracted file name will be stored.
 * @param size Pointer to an integer where the extracted file size will be stored.
 */
void extractFileInfo(uint8_t* packet_data, char* getname, int* size);

/**
 * @brief Processes the header packet of a Ymodem transfer.
 *
 * This function extracts information from the header packet, such as the file name and size,
 * and performs validation checks.
 *
 * @param packet_data Pointer to the packet data.
 * @param packet_length Length of the packet data.
 * @param maxsize Maximum allowed size for the file.
 * @param getname Pointer to a buffer where the file name will be stored.
 * @param size Pointer to an integer where the file size will be stored.
 * @param errors Pointer to an unsigned integer where the error count will be stored.
 * @return ReceivePacketStatus Status of the packet processing.
 *         - PACKET_OK: Packet processed successfully.
 *         - PACKET_SIZE_OVERFLOW: File size exceeds the maximum allowed size.
 *         - PACKET_SIZE_NULL: File size is null or invalid.
 *         - PACKET_MAX_ERRORS: Maximum number of errors reached.
 */
ReceivePacketStatus processHeaderPacket(uint8_t* packet_data, int packet_length, unsigned int maxsize, char* getname, int* size,
                                        unsigned int* errors);

/**
 * @brief Processes a received Ymodem packet.
 *
 * This function handles the processing of a Ymodem packet, including writing data to a file,
 * managing packet counts, and handling errors.
 *
 * @param packet_data Pointer to the data of the received packet.
 * @param packet_length Length of the received packet.
 * @param ffd Reference to the file object where data will be written.
 * @param maxsize Maximum allowed size of the file.
 * @param getname Pointer to a buffer where the filename will be stored.
 * @param packets_received Number of packets received so far.
 * @param size Pointer to an integer where the size of the file will be stored.
 * @param file_done Pointer to an unsigned integer that indicates if the file transfer is complete.
 * @param errors Pointer to an unsigned integer that counts the number of errors encountered.
 *
 * @return An integer indicating the status of the packet processing.
 *         0 indicates success, while non-zero values indicate different error conditions.
 */
ReceivePacketStatus processPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int maxsize, char* getname,
                                  unsigned int packets_received, int* size, unsigned int* file_done, unsigned int* errors);

/**
 * @brief Handles a file session for receiving data.
 *
 * @param ffd Reference to the file object where the received data will be stored.
 * @param maxsize Maximum size of the file to be received.
 * @param getname Pointer to a character array where the name of the received file will be stored.
 * @param session_done Pointer to an unsigned int that will be set to 1 if the session is completed successfully, 0 otherwise.
 * @param errors Pointer to an unsigned int that will be incremented if any errors occur during the session.
 * @return int Status code indicating the result of the file session handling.
 */
int handleFileSession(fs::File& ffd, unsigned int maxsize, char* getname, unsigned int* session_done, unsigned int* errors);

#endif // YMODEMRECEIVE_H
