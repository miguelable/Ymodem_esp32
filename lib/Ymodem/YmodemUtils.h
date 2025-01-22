#ifndef YMODEMUTILS_H
#define YMODEMUTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "rom/crc.h"
#include <FS.h>
#include <driver/uart.h>

#include "YmodemDef.h"

void IRAM_ATTR LED_toggle();
unsigned short crc16(const unsigned char* buf, unsigned long count);
int32_t        Receive_Byte(unsigned char* c, uint32_t timeout);
void           uart_consume();
uint32_t       Send_Byte(char c);
void           send_CA();
void           send_ACK();
void           send_ACKCRC16();
void           send_NAK();
void           send_CRC16();
int32_t        Receive_Packet(uint8_t* data, int* length, uint32_t timeout);

#endif // YMODEMUTILS_H