
#ifndef YMODEMCORE_H
#define YMODEMCORE_H

#include "YmodemDef.h"
#include "YmodemReceive.h"
// #include "YmodemTransmit.h"
#include "YmodemPaquets.h"
#include "YmodemUtils.h"


class Ymodem
{
public:
  Ymodem();
  ~Ymodem(){};

  int receive(fs::File& ffd, unsigned int maxsize, char* getname);
  int transmit(char* sendFileName, unsigned int sizeFile, fs::File& ffd);

  void setLedPin(int pin);
  int  getLedPin();

private:
  int  ledPin = YMODEM_LED_ACT;
  void finalizeSession();
};

#endif // YMODEMCORE_H