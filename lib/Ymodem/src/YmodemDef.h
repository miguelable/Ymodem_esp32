/**
 * @file YmodemDef.h
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem protocol definitions and configuration
 * @version 0.1
 * @date 2025-01-24
 *
 * This file contains the definitions and configuration parameters for the Ymodem protocol.
 * These parameters include packet sizes, control characters, timeouts, and other settings
 * used in the Ymodem protocol implementation.
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __YMODEMDEFINES_H__
#define __YMODEMDEFINES_H__
// === UART DEFINES ====
#define EX_UART_NUM UART_NUM_1 /*!< UART port number */
#define BUF_SIZE (1080)        /*!< UART buffer size */
#define MAX_BUFFER_SIZE (1024) /*!< Maximum buffer size */

// === LED pin used to show transfer activity ===
// === Set to 0 if you don't want to use it   ===
#define YMODEM_LED_ACT 0    /*!< GPIO pin number for activity LED, set to 0 if you dont have LED*/
#define YMODEM_LED_ACT_ON 1 /*!< LED ON state, 1 for active high, 0 for active low */

#define YMODEM_RX_PIN GPIO_NUM_14 /*!< RX pin number */
#define YMODEM_TX_PIN GPIO_NUM_33 /*!< TX pin number */
#define YMODEM_LED_PIN GPIO_NUM_2 /*!< LED pin number */

// ==== Y-MODEM defines ====
#define PACKET_SEQNO_INDEX (1)                           /*!< Packet sequence number index */
#define PACKET_SEQNO_COMP_INDEX (2)                      /*!< Packet sequence number complement index */
#define PACKET_HEADER (3)                                /*!< Packet header size */
#define PACKET_TRAILER (2)                               /*!< Packet trailer size */
#define PACKET_OVERHEAD (PACKET_HEADER + PACKET_TRAILER) /*!< Packet overhead size */
#define PACKET_SIZE (128)                                /*!< Packet data size */
#define PACKET_1K_SIZE (1024)                            /*!< Packet 1K data size */
#define FILE_SIZE_LENGTH (16)                            /*!< File size length */

#define SOH (0x01)   /*!< start of 128-byte data packet */
#define STX (0x02)   /*!< start of 1024-byte data packet */
#define EOT (0x04)   /*!< end of transmission */
#define ACK (0x06)   /*!< acknowledge */
#define NAK (0x15)   /*!< negative acknowledge */
#define CA (0x18)    /*!< two of these in succession aborts transfer */
#define CRC16 (0x43) /*!< 'C' == 0x43, request 16-bit CRC */

#define ABORT1 (0x41) /*!< 'A' == 0x41, abort by sender */
#define ABORT2 (0x61) /*!< 'a' == 0x61, abort by receiver */

#define NAK_TIMEOUT (1000) /*!< Timeout for NAK response */
#define MAX_ERRORS (100)   /*!< Maximum number of errors allowed */

#define YM_MAX_FILESIZE (10 * 1024 * 1024) /*!< Maximum file size allowed */
#define PROGRESS_BAR_WIDTH (50)            /*!< Progress bar width in characters */

#ifdef YMODEM_LSM1X0A
#define YMODEM_RESET_PIN GPIO_NUM_15 /*!< Reset LSM1X0A Modem pin number */
#endif

#endif // __YMODEMDEFINES_H__