/**
 * @file YmodemReceive.cpp
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief Ymodem Packet reception functions
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
#include "YmodemReceive.h"

ReceivePacketStatus processDataPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int file_size)
{
  static unsigned int file_len = 0;

  if (file_len < file_size) {
    unsigned int write_len = packet_length;
    file_len += packet_length;
    if (file_len > file_size) {
      write_len -= (file_len - file_size);
      file_len = file_size;
    }

    int written_bytes = ffd.write(packet_data + PACKET_HEADER, write_len);
    if (written_bytes != write_len) {
      send_CA();
      return PACKET_ERROR_WRITING;
    }
    LED_toggle();
  }
  send_ACK();
  return PACKET_RECEIVED_OK;
}

void handleEOFPacket(unsigned int* file_done, unsigned int* errors)
{
  static int eof_cnt = 0;

  eof_cnt++;
  if (eof_cnt == 1) {
    send_NAK();
  }
  else {
    send_ACK();
    *file_done = 1;
  }
}

void extractFileInfo(uint8_t* packet_data, char* getname, int* size)
{
  char         file_size[128];
  unsigned int i        = 0;
  uint8_t*     file_ptr = packet_data + PACKET_HEADER;

  // Extraer el nombre del archivo
  if (getname) {
    while ((*file_ptr != 0) && (i < 64)) { // Máximo 64 caracteres para el nombre
      *getname = *file_ptr++;
      getname++;
      i++;
    }
    *getname = '\0'; // Terminar la cadena
  }

  // Saltar el nombre del archivo y buscar el tamaño
  while ((*file_ptr != 0) && (file_ptr < packet_data + PACKET_1K_SIZE)) {
    file_ptr++;
  }
  file_ptr++; // Saltar el byte nulo después del nombre

  // Extraer el tamaño del archivo
  i = 0;
  while ((*file_ptr != ' ') && (i < sizeof(file_size) - 1)) { // Leer hasta el espacio
    file_size[i++] = *file_ptr++;
  }
  file_size[i] = '\0'; // Terminar la cadena

  // Convertir el tamaño del archivo de texto a entero
  if (strlen(file_size) > 0) {
    *size = strtol(file_size, NULL, 10); // Base 10
  }
  else {
    *size = 0; // Si no hay tamaño, se interpreta como 0
  }
}

ReceivePacketStatus processHeaderPacket(uint8_t* packet_data, int packet_length, unsigned int maxsize, char* getname, int* size, unsigned int* errors)
{
  if (packet_data[PACKET_HEADER] != 0) { // Paquete válido
    extractFileInfo(packet_data, getname, size);
    if (*size < 1 || *size > maxsize) {
      send_CA();
      return (*size > maxsize) ? PACKET_SIZE_OVERFLOW : PACKET_SIZE_NULL;
    }
    send_ACKCRC16();
    return PACKET_RECEIVED_OK;
  }
  else { // Paquete de encabezado vacío
    (*errors)++;
    if (*errors > 5) {
      send_CA();
      return PACKET_MAX_ERRORS;
    }
    send_NAK();
    return PACKET_RECEIVED_OK;
  }
}

ReceivePacketStatus processPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int maxsize, char* getname,
                                  unsigned int packets_received, int* size, unsigned int* file_done, unsigned int* errors)
{
  if (packet_length == 0) { // Paquete EOF
    handleEOFPacket(file_done, errors);
    return PACKET_RECEIVED_OK;
  }
  else if (packet_length == -1) { // Abortado por transmisor
    send_ACK();
    return PACKET_ABORTED;
  }
  else if (packet_length == -2) { // Error de recepción
    (*errors)++;
    if (*errors > 5) {
      send_CA();
      return PACKET_MAX_ERRORS;
    }
    send_NAK();
    return PACKET_RECEIVED_OK;
  }

  // Paquete normal
  if (packets_received == 0) {
    return processHeaderPacket(packet_data, packet_length, maxsize, getname, size, errors);
  }
  else {
    return processDataPacket(packet_data, packet_length, ffd, *size);
  }
}

int handleFileSession(fs::File& ffd, unsigned int maxsize, char* getname, unsigned int* session_done, unsigned int* errors)
{
  unsigned int file_done = 0, packets_received = 0;
  int          size = 0;

  while (!file_done) {
    LED_toggle();
    int     packet_length = 0;
    uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];

    ReceivePacketStatus result = ReceiveAndValidatePacket(packet_data, &packet_length, NAK_TIMEOUT);
    if (result == PACKET_RECEIVED_OK) {
      int process_result = processPacket(packet_data, packet_length, ffd, maxsize, getname, packets_received, &size, &file_done, errors);
      if (process_result != PACKET_RECEIVED_OK) {
        return process_result; // Error durante el procesamiento
      }
      packets_received++;
    }
    else if (result == PACKET_ABORTED) {
      send_CA();
      return PACKET_ABORTED;
    }
    else { // Timeout o error
      (*errors)++;
      if (*errors > MAX_ERRORS) {
        send_CA();
        return PACKET_MAX_ERRORS;
      }
      send_CRC16();
    }
  }

  *session_done = 1;
  return size;
}