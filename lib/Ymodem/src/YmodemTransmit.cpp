/**
 * @file YmodemTransmit.cpp
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
#include "YmodemTransmit.h"

YmodemPacketStatus waitForReceiverResponse()
{
  unsigned char receivedC;
  int           err = 0;

  do {
    send_CRC16();
    LED_toggle();
  } while (Receive_Byte(&receivedC, NAK_TIMEOUT) != BYTE_OK && err++ < MAX_ERRORS);

  if (err >= MAX_ERRORS) {
    send_CA();
    return YMODEM_TIMEOUT;
  }
  else if (receivedC != CRC16) {
    send_CA();
    return YMODEM_CRC_ERROR;
  }

  return YMODEM_TRANSMIT_START;
}

YmodemPacketStatus sendInitialPacket(const char* sendFileName, unsigned int sizeFile)
{
  uint8_t            packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  YmodemPacketStatus err;

  Ymodem_PrepareIntialPacket(packet_data, sendFileName, sizeFile);
  do {
    // Send Packet
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);

    // Wait for Ack
    err = Ymodem_WaitResponse(ACK);
    if (err == YMODEM_TIMEOUT || err == YMODEM_INVALID_HEADER) {
      send_CA();
      return err;
    }
    else if (err == YMODEM_ABORTED_BY_SENDER)
      return err; // abort
    LED_toggle();
  } while (err != YMODEM_RECEIVED_CORRECT);

  // After initial block the receiver sends 'C' after ACK
  err = Ymodem_WaitResponse(CRC16);
  if (err != YMODEM_RECEIVED_CORRECT) {
    send_CA();
    return err;
  }

  return YMODEM_RECEIVED_OK; // Success
}

YmodemPacketStatus readFileBlock(const char* fileName, FileSystem& fs, uint8_t* buffer, size_t& fileSize, size_t offset)
{
  size_t bytesToRead = std::min(fileSize, static_cast<size_t>(PACKET_1K_SIZE));
  int    err         = fs.readFromFile(fileName, buffer, bytesToRead, offset);
  if (err != LITTLEFS_OK) {
    const char* errorMsg = "Failed to read file\n";
    uart_write_bytes(UART_NUM_0, errorMsg, strlen(errorMsg));
    send_CA();
    return YMODEM_READ_ERROR; // Error al leer el archivo
  }
  return YMODEM_READ_FILE_OK;
}

void displayProgress(size_t offset, size_t totalSize, unsigned long startTime)
{
  int progress = (offset * 100) / totalSize;
  int filled   = (offset * PROGRESS_BAR_WIDTH) / totalSize;

  unsigned long elapsedTime        = millis() - startTime;
  unsigned long estimatedTotalTime = 0;
  unsigned long remainingTime      = 0;

  if (offset > 0 && offset < totalSize) {
    estimatedTotalTime = (elapsedTime * totalSize) / offset;
    remainingTime      = (estimatedTotalTime > elapsedTime) ? (estimatedTotalTime - elapsedTime) / 1000 : 0;
  }

  char progressBar[124];
  snprintf(progressBar, sizeof(progressBar), "Progress: [");
  uart_write_bytes(UART_NUM_0, progressBar, strlen(progressBar));

  for (int i = 0; i < PROGRESS_BAR_WIDTH; i++) {
    uart_write_bytes(UART_NUM_0, (i < filled) ? "\033[42m \033[0m" : "\033[41m \033[0m", 9);
  }

  unsigned long minutes = remainingTime / 60;
  unsigned long seconds = remainingTime % 60;
  snprintf(progressBar, sizeof(progressBar), "\033[0m] %d%% Time: %lum %lus  \r", progress, minutes, seconds);
  uart_write_bytes(UART_NUM_0, progressBar, strlen(progressBar));
}

YmodemPacketStatus sendPacketAndHandleResponse(uint8_t* packet_data, uint16_t& blkNumber, size_t& fileSize, size_t& offset, size_t totalSize,
                                               unsigned long startTime)
{
  YmodemPacketStatus err;
  size_t             bytesToRead = std::min(fileSize, static_cast<size_t>(PACKET_1K_SIZE));

  do {
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD);
    err = Ymodem_WaitResponse(ACK);

    if (err == YMODEM_RECEIVED_CORRECT) {
      offset += bytesToRead;   // Mover el offset al siguiente bloque
      fileSize -= bytesToRead; // Reducir el tamaño restante
      displayProgress(offset, totalSize, startTime);
    }
    else if (err == YMODEM_TIMEOUT || err == YMODEM_INVALID_HEADER) {
      send_CA();
      return err; // Timeout o respuesta incorrecta
    }
    else if (err == YMODEM_ABORTED_BY_SENDER) {
      return err; // Abort
    }
  } while (err != YMODEM_RECEIVED_CORRECT);

  LED_toggle(); // Indicar progreso con el LED
  return YMODEM_RECEIVED_OK;
}

YmodemPacketStatus sendFileBlocks(const char* fileName, FileSystem& fs)
{
  uint8_t  packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  uint8_t  buffer[PACKET_1K_SIZE];
  uint16_t blkNumber = 0x01;
  size_t   offset    = 0;
  size_t   fileSize  = fs.getFileSize(fileName);
  size_t   totalSize = fileSize;

  unsigned long startTime = millis(); // Tiempo de inicio de la transferencia

  while (fileSize > 0) {

    // Leer datos del archivo en bloques
    YmodemPacketStatus err = readFileBlock(fileName, fs, buffer, fileSize, offset);
    if (err != YMODEM_READ_FILE_OK) {
      return err; // Error al leer el bloque
    }

    // Preparar el paquete con los datos leídos
    Ymodem_PreparePacket(packet_data, blkNumber, std::min(fileSize, static_cast<size_t>(PACKET_1K_SIZE)), buffer);

    // Enviar el paquete y manejar la respuesta
    err = sendPacketAndHandleResponse(packet_data, blkNumber, fileSize, offset, totalSize, startTime);
    if (err != YMODEM_RECEIVED_OK) {
      return err; // Error al enviar el paquete
    }
    blkNumber++;
  }
  uart_write_bytes(UART_NUM_0, "\n", 1); // Finalizar con una nueva línea
  return YMODEM_TRANSMIT_OK;             // Éxito
}

YmodemPacketStatus sendEOT()
{
  YmodemPacketStatus err;

  send_EOT();
  do {
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK);
    if (err == YMODEM_RECEIVED_NAK) { // NAK
      send_EOT();
    }
    else if (err == YMODEM_TIMEOUT || err == YMODEM_INVALID_HEADER) {
      send_CA();
      return err; // timeout or wrong response
    }
    else if (err == YMODEM_ABORTED_BY_SENDER)
      return err; // abort
  } while (err != YMODEM_RECEIVED_CORRECT);

  return YMODEM_RECEIVED_OK; // Success
}

YmodemPacketStatus sendLastPacket()
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];

  YmodemPacketStatus err = Ymodem_WaitResponse(CRC16);
  if (err != YMODEM_RECEIVED_CORRECT) {
    send_CA();
    return err;
  }

  LED_toggle();
  Ymodem_PrepareLastPacket(packet_data);
  do {
    // Send Packet
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK);
    if (err == YMODEM_TIMEOUT || err == YMODEM_INVALID_HEADER) {
      send_CA();
      return err; // timeout or wrong response
    }
    else if (err == YMODEM_ABORTED_BY_SENDER)
      return err; // abort
  } while (err != YMODEM_RECEIVED_CORRECT);

#if YMODEM_LED_ACT
  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
#endif

  return YMODEM_RECEIVED_OK; // Success
}