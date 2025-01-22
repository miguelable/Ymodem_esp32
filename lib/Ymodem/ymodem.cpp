/*
 * ESP32 YModem driver
 *
 * Copyright (C) LoBo 2017
 *
 * Author: Boris Lovosevic (loboris@gmail.com)
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#include "ymodem.h"
#include "driver/gpio.h"
#include "esp_spiffs.h"
#include "rom/crc.h"
#include <driver/uart.h>

//----------------------------------
static void IRAM_ATTR LED_toggle()
{
#if YMODEM_LED_ACT
  if (GPIO.out & (1 << YMODEM_LED_ACT)) {
    GPIO.out_w1tc = (1 << YMODEM_LED_ACT);
  }
  else {
    GPIO.out_w1ts = (1 << YMODEM_LED_ACT);
  }
#endif
}

//------------------------------------------------------------------------
static unsigned short crc16(const unsigned char* buf, unsigned long count)
{
  unsigned short crc = 0;
  int            i;

  while (count--) {
    crc = crc ^ *buf++ << 8;

    for (i = 0; i < 8; i++) {
      if (crc & 0x8000)
        crc = crc << 1 ^ 0x1021;
      else
        crc = crc << 1;
    }
  }
  return crc;
}

//--------------------------------------------------------------
static int32_t Receive_Byte(unsigned char* c, uint32_t timeout)
{
  unsigned char ch;
  int           len = uart_read_bytes(EX_UART_NUM, &ch, 1, timeout / portTICK_RATE_MS);
  if (len <= 0)
    return -1;

  *c = ch;
  return 0;
}

//------------------------
static void uart_consume()
{
  uint8_t ch[64];
  while (uart_read_bytes(EX_UART_NUM, ch, 64, 100 / portTICK_RATE_MS) > 0)
    ;
}

//--------------------------------
static uint32_t Send_Byte(char c)
{
  uart_write_bytes(EX_UART_NUM, &c, 1);
  return 0;
}

//----------------------------
static void send_CA()
{
  Send_Byte(CA);
  Send_Byte(CA);
}

//-----------------------------
static void send_ACK()
{
  Send_Byte(ACK);
}

//----------------------------------
static void send_ACKCRC16()
{
  Send_Byte(ACK);
  Send_Byte(CRC16);
}

//-----------------------------
static void send_NAK()
{
  Send_Byte(NAK);
}

//-------------------------------
static void send_CRC16()
{
  Send_Byte(CRC16);
}

//--------------------------------------------------------------------------
static int32_t Receive_Packet(uint8_t* data, int* length, uint32_t timeout)
{
  int           count, packet_size, i;
  unsigned char ch;
  *length = 0;

  // receive 1st byte
  if (Receive_Byte(&ch, timeout) < 0) {
    return -1;
  }

  switch (ch) {
    case SOH:
      packet_size = PACKET_SIZE;
      break;
    case STX:
      packet_size = PACKET_1K_SIZE;
      break;
    case EOT:
      *length = 0;
      return 0;
    case CA:
      if (Receive_Byte(&ch, timeout) < 0) {
        return -2;
      }
      if (ch == CA) {
        *length = -1;
        return 0;
      }
      else
        return -1;
    case ABORT1:
    case ABORT2:
      return -2;
    default:
      vTaskDelay(100 / portTICK_RATE_MS);
      uart_consume();
      return -1;
  }

  *data         = (uint8_t)ch;
  uint8_t* dptr = data + 1;
  count         = packet_size + PACKET_OVERHEAD - 1;

  for (i = 0; i < count; i++) {
    if (Receive_Byte(&ch, timeout) < 0) {
      return -1;
    }
    *dptr++ = (uint8_t)ch;
    ;
  }

  if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) {
    *length = -2;
    return 0;
  }
  if (crc16(&data[PACKET_HEADER], packet_size + PACKET_TRAILER) != 0) {
    *length = -2;
    return 0;
  }

  *length = packet_size;
  return 0;
}

//------------------------------------------------------------------------------------
static void Ymodem_PrepareIntialPacket(uint8_t* data, char* fileName, uint32_t length)
{
  uint16_t tempCRC;

  memset(data, 0, PACKET_SIZE + PACKET_HEADER);
  // Make first three packet
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;

  // add filename
  sprintf((char*)(data + PACKET_HEADER), "%s", fileName);

  // add file site
  sprintf((char*)(data + PACKET_HEADER + strlen((char*)(data + PACKET_HEADER)) + 1), "%d", length);
  data[PACKET_HEADER + strlen((char*)(data + PACKET_HEADER)) + 1 +
       strlen((char*)(data + PACKET_HEADER + strlen((char*)(data + PACKET_HEADER)) + 1))] = ' ';

  // add crc
  tempCRC                               = crc16(&data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER]     = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-------------------------------------------------
static void Ymodem_PrepareLastPacket(uint8_t* data)
{
  uint16_t tempCRC;

  memset(data, 0, PACKET_SIZE + PACKET_HEADER);
  data[0] = SOH;
  data[1] = 0x00;
  data[2] = 0xff;
  tempCRC = crc16(&data[PACKET_HEADER], PACKET_SIZE);
  // tempCRC = crc16_le(0, &data[PACKET_HEADER], PACKET_SIZE);
  data[PACKET_SIZE + PACKET_HEADER]     = tempCRC >> 8;
  data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-----------------------------------------------------------------------------------------
static void Ymodem_PreparePacket(uint8_t* data, uint8_t pktNo, uint32_t sizeBlk, fs::File& ffd)
{
  uint16_t i, size;
  uint16_t tempCRC;

  data[0] = STX;
  data[1] = (pktNo & 0x000000ff);
  data[2] = (~(pktNo & 0x000000ff));

  size = sizeBlk < PACKET_1K_SIZE ? sizeBlk : PACKET_1K_SIZE;
  // Read block from file
  if (size > 0) {
    size = ffd.read(data + PACKET_HEADER, size);
  }

  if (size < PACKET_1K_SIZE) {
    for (i = size + PACKET_HEADER; i < PACKET_1K_SIZE + PACKET_HEADER; i++) {
      data[i] = 0x00; // EOF (0x1A) or 0x00
    }
  }
  tempCRC = crc16(&data[PACKET_HEADER], PACKET_1K_SIZE);
  // tempCRC = crc16_le(0, &data[PACKET_HEADER], PACKET_1K_SIZE);
  data[PACKET_1K_SIZE + PACKET_HEADER]     = tempCRC >> 8;
  data[PACKET_1K_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-------------------------------------------------------------
static uint8_t Ymodem_WaitResponse(uint8_t ackchr, uint8_t tmo)
{
  unsigned char receivedC;
  uint32_t      errors = 0;

  do {
    if (Receive_Byte(&receivedC, NAK_TIMEOUT) == 0) {
      if (receivedC == ackchr) {
        return 1;
      }
      else if (receivedC == CA) {
        send_CA();
        return 2; // CA received, Sender abort
      }
      else if (receivedC == NAK) {
        return 3;
      }
      else {
        return 4;
      }
    }
    else {
      errors++;
    }
  } while (errors < tmo);
  return 0;
}

Ymodem::Ymodem()
{
  pinMode(ledPin, OUTPUT);
}

void Ymodem::setLedPin(int pin)
{
  ledPin = pin;
  // Set the LED pin as an output
  pinMode(ledPin, OUTPUT);
  log_i("LED pin set to %d", ledPin);
}

int Ymodem::getLedPin()
{
  return ledPin;
}

int Ymodem::transmit(char* sendFileName, unsigned int sizeFile, fs::File& ffd)
{
  uint8_t       packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  uint16_t      blkNumber;
  unsigned char receivedC;
  int           i, err;
  uint32_t      size = 0;

  // Wait for response from receiver
  err = 0;
  do {
    Send_Byte(CRC16);
    LED_toggle();
  } while (Receive_Byte(&receivedC, NAK_TIMEOUT) < 0 && err++ < 45);

  if (err >= 45 || receivedC != CRC16) {
    send_CA();
    return -1;
  }

  // === Prepare first block and send it =======================================
  /* When the receiving program receives this block and successfully
   * opened the output file, it shall acknowledge this block with an ACK
   * character and then proceed with a normal YMODEM file transfer
   * beginning with a "C" or NAK tranmsitted by the receiver.
   */
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

  // === Send file blocks ======================================================
  size      = sizeFile;
  blkNumber = 0x01;

  // Resend packet if NAK  for a count of 10 else end of communication
  while (size) {
    // Prepare and send next packet
    Ymodem_PreparePacket(packet_data, blkNumber, size, ffd);
    do {
      // Send Packet
      uart_write_bytes(EX_UART_NUM, (char*)packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD);

      // Wait for Ack
      err = Ymodem_WaitResponse(ACK, 10);
      if (err == 1) {
        blkNumber++;
        if (size > PACKET_1K_SIZE)
          size -= PACKET_1K_SIZE; // Next packet
        else
          size = 0; // Last packet sent
      }
      else if (err == 0 || err == 4) {
        send_CA();
        return -4; // timeout or wrong response
      }
      else if (err == 2)
        return -5; // abort
    } while (err != 1);
    LED_toggle();
  }

  // === Send EOT ==============================================================
  Send_Byte(EOT); // Send (EOT)
  // Wait for Ack
  do {
    // Wait for Ack
    err = Ymodem_WaitResponse(ACK, 10);
    if (err == 3) {   // NAK
      Send_Byte(EOT); // Send (EOT)
    }
    else if (err == 0 || err == 4) {
      send_CA();
      return -6; // timeout or wrong response
    }
    else if (err == 2)
      return -7; // abort
  } while (err != 1);

  // === Receiver requests next file, prepare and send last packet =============
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
  return 0; // file transmitted successfully
}

int processDataPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int file_size, unsigned int* errors)
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
      return -6;
    }
    LED_toggle();
  }
  send_ACK();
  return 0;
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

int processHeaderPacket(uint8_t* packet_data, int packet_length, unsigned int maxsize, char* getname, int* size, unsigned int* errors)
{
  if (packet_data[PACKET_HEADER] != 0) { // Paquete válido
    extractFileInfo(packet_data, getname, size);
    if (*size < 1 || *size > maxsize) {
      send_CA();
      return (*size > maxsize) ? -9 : -4;
    }
    send_ACKCRC16();
    return 0;
  }
  else { // Paquete de encabezado vacío
    (*errors)++;
    if (*errors > 5) {
      send_CA();
      return -5;
    }
    send_NAK();
    return 0;
  }
}

void Ymodem::finalizeSession()
{
#if YMODEM_LED_ACT
  digitalWrite(ledPin, YMODEM_LED_ACT_ON ^ 1);
#endif
}

int processPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int maxsize, char* getname, unsigned int packets_received,
                  int* size, unsigned int* file_done, unsigned int* errors)
{
  if (packet_length == 0) { // Paquete EOF
    handleEOFPacket(file_done, errors);
    return 0;
  }
  else if (packet_length == -1) { // Abortado por transmisor
    send_ACK();
    return -1;
  }
  else if (packet_length == -2) { // Error de recepción
    (*errors)++;
    if (*errors > 5) {
      send_CA();
      return -2;
    }
    send_NAK();
    return 0;
  }

  // Paquete normal
  if (packets_received == 0) {
    return processHeaderPacket(packet_data, packet_length, maxsize, getname, size, errors);
  }
  else {
    return processDataPacket(packet_data, packet_length, ffd, *size, errors);
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

    int result = Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT);
    if (result == 0) { // Paquete recibido correctamente
      int process_result = processPacket(packet_data, packet_length, ffd, maxsize, getname, packets_received, &size, &file_done, errors);
      if (process_result < 0) {
        return process_result; // Error durante el procesamiento
      }
      packets_received++;
    }
    else if (result == -2) { // Usuario abortó
      send_CA();
      return -7;
    }
    else { // Timeout o error
      (*errors)++;
      if (*errors > MAX_ERRORS) {
        send_CA();
        return -8;
      }
      send_CRC16();
    }
  }

  *session_done = 1;
  return size;
}

/**
 * @brief Receives a file using the Ymodem protocol.
 *
 * This function handles the reception of a file over a Ymodem connection.
 * It processes packets, handles errors, and writes the received data to the specified file.
 *
 * @param ffd Reference to the file object where the received data will be written.
 * @param maxsize Maximum allowed size of the file to be received.
 * @param getname Pointer to a character array where the received file name will be stored.
 * @return int The size of the received file in bytes, or a negative error code:
 *         - -1: Abort by sender
 *         - -2: Too many errors
 *         - -3: Packet sequence error
 *         - -4: Invalid file size
 *         - -5: Filename packet error
 *         - -6: File write error
 *         - -7: User abort
 *         - -8: Timeout
 *         - -9: File size exceeds maxsize
 */
int Ymodem::receive(fs::File& ffd, unsigned int maxsize, char* getname)
{
  int          size         = 0;
  unsigned int session_done = 0, errors = 0;

  while (!session_done) {
    int result = handleFileSession(ffd, maxsize, getname, &session_done, &errors);
    if (result < 0) {
      size = result; // Código de error
      break;
    }
    size = result; // Tamaño del archivo recibido
  }

  finalizeSession();
  return size;
}