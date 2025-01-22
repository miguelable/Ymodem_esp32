#ifndef YMODEMRECEIVE_H
#define YMODEMRECEIVE_H

#include "YmodemDef.h"
#include "YmodemUtils.h"

int  processDataPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int file_size, unsigned int* errors);
void handleEOFPacket(unsigned int* file_done, unsigned int* errors);
void extractFileInfo(uint8_t* packet_data, char* getname, int* size);
int  processHeaderPacket(uint8_t* packet_data, int packet_length, unsigned int maxsize, char* getname, int* size, unsigned int* errors);
int  processPacket(uint8_t* packet_data, int packet_length, fs::File& ffd, unsigned int maxsize, char* getname, unsigned int packets_received,
                   int* size, unsigned int* file_done, unsigned int* errors);
int  handleFileSession(fs::File& ffd, unsigned int maxsize, char* getname, unsigned int* session_done, unsigned int* errors);

#endif // YMODEMRECEIVE_H
