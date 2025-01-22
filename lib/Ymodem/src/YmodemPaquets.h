#ifndef YMODEMPAQUETS_H
#define YMODEMPAQUETS_H

#include "YmodemDef.h"
#include "YmodemUtils.h"

void    Ymodem_PrepareIntialPacket(uint8_t* data, char* fileName, uint32_t length);
void    Ymodem_PrepareLastPacket(uint8_t* data);
void    Ymodem_PreparePacket(uint8_t* data, uint8_t pktNo, uint32_t sizeBlk, fs::File& ffd);
uint8_t Ymodem_WaitResponse(uint8_t ackchr, uint8_t tmo);

#endif // YMODEMPAQUETS_H