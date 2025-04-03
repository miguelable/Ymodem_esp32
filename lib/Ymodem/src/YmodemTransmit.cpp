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

int waitForReceiverResponse()
{
  unsigned char receivedC;
  int           err = 0;

  do {
    send_CRC16();
    LED_toggle();
  } while (Receive_Byte(&receivedC, NAK_TIMEOUT) < 0 && err++ < 45);

  if (err >= 45 || receivedC != CRC16) {
    send_CA();
    return -1;
  }

  return 0;
}

int sendInitialPacket(const char* sendFileName, unsigned int sizeFile)
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  int     err;

  Ymodem_PrepareIntialPacket(packet_data, sendFileName, sizeFile);
  do {
    // Send Packet
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);

    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return -2; // timeout or wrong response
    }
    else if (err == 2)
      return 98; // abort
    LED_toggle();
  } while (err != 1);

  // After initial block the receiver sends 'C' after ACK
  if (Ymodem_WaitResponse(CRC16, 10) != 1) {
    send_CA();
    return -3;
  }

  return 0;
}

int readFileBlock(const char* fileName, FileSystem& fs, uint8_t* buffer, size_t& fileSize, size_t offset)
{
  size_t bytesToRead = std::min(fileSize, static_cast<size_t>(PACKET_1K_SIZE));
  int    err         = fs.readFromFile(fileName, buffer, bytesToRead, offset);
  if (err != LITTLEFS_OK) {
    const char* errorMsg = "Failed to read file\n";
    uart_write_bytes(UART_NUM_0, errorMsg, strlen(errorMsg));
    send_CA();
    return -2; // Error al leer el archivo
  }
  return 0;
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

  char progressBar[512];
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

int sendPacketAndHandleResponse(uint8_t* packet_data, uint16_t& blkNumber, size_t& fileSize, size_t& offset, size_t totalSize,
                                unsigned long startTime)
{
  int    err;
  size_t bytesToRead = std::min(fileSize, static_cast<size_t>(PACKET_1K_SIZE));

  do {
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD);
    err = Ymodem_WaitResponse(ACK, 10);

    if (err == 1) {
      offset += bytesToRead;   // Mover el offset al siguiente bloque
      fileSize -= bytesToRead; // Reducir el tamaño restante
      displayProgress(offset, totalSize, startTime);
    }
    else if (err == 0 || err == 4) {
      send_CA();
      return -3; // Timeout o respuesta incorrecta
    }
    else if (err == 2) {
      return -4; // Abort
    }
  } while (err != 1);

  LED_toggle(); // Indicar progreso con el LED
  return 0;
}

int sendFileBlocks(const char* fileName, FileSystem& fs)
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
    int err = readFileBlock(fileName, fs, buffer, fileSize, offset);
    if (err != 0) {
      return err; // Error al leer el bloque
    }

    // Preparar el paquete con los datos leídos
    Ymodem_PreparePacket(packet_data, blkNumber, std::min(fileSize, static_cast<size_t>(PACKET_1K_SIZE)), buffer);

    // Enviar el paquete y manejar la respuesta
    err = sendPacketAndHandleResponse(packet_data, blkNumber, fileSize, offset, totalSize, startTime);
    if (err != 0) {
      return err; // Error al enviar el paquete
    }
    blkNumber++;
  }
  uart_write_bytes(UART_NUM_0, "\n", 1); // Finalizar con una nueva línea
  return 0;                              // Éxito
}

int sendEOT()
{
  int err;

  send_EOT();
  do {
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 3) { // NAK
      send_EOT();
    }
    else if (err == 0 || err == 4) {
      send_CA();
      return -6; // timeout or wrong response
    }
    else if (err == 2)
      return -7; // abort
  } while (err != 1);

  return 0;
}

int sendLastPacket()
{
  uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  int     err;

  if (Ymodem_WaitResponse(CRC16, 10) != 1) {
    send_CA();
    return -8;
  }

  LED_toggle();
  Ymodem_PrepareLastPacket(packet_data);
  do {
    // Send Packet
    uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_SIZE + PACKET_OVERHEAD);
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 0 || err == 4) {
      send_CA();
      return -9; // timeout or wrong response
    }
    else if (err == 2)
      return -10; // abort
  } while (err != 1);

#if YMODEM_LED_ACT
  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);
#endif

  return 0;
}