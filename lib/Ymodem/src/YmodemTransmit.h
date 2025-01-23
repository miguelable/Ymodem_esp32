#ifndef YMODEMTRANSMIT_H
#define YMODEMTRANSMIT_H

#include "YmodemDef.h"
#include "YmodemPaquets.h"
#include "YmodemUtils.h"

int waitForReceiverResponse();
int sendInitialPacket(char* sendFileName, unsigned int sizeFile);
int sendFileBlocks(unsigned int sizeFile, fs::File& ffd);
int sendEOT();
int sendLastPacket();

#endif // YMODEMTRANSMIT_H