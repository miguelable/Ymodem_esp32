/**
 * @file YmodemCore.h
 * @author Miguel Ferrer (mferrer@inbiot.es)
 * @brief  Ymodem core functions
 * @version 0.1
 * @date 2025-01-24
 *
 * This file contains the core functions for the Ymodem protocol, including
 * both file transmission and reception. It provides a high-level interface
 * for sending and receiving files using the Ymodem protocol.
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef YMODEMCORE_H
#define YMODEMCORE_H

#include "YmodemReceive.h"
#include "YmodemTransmit.h"

/**
 * @brief  Ymodem class
 *
 * This class provides high-level functions for sending and receiving files
 * using the Ymodem protocol. It encapsulates the low-level packet processing
 * and communication functions to simplify the file transfer process.
 *
 */
class Ymodem
{
public:
  /**
   * @brief Constructor for the Ymodem class.
   *
   * Initializes a new instance of the Ymodem class.
   */
  Ymodem();

  /**
   * @brief Destructor for the Ymodem class.
   *
   * This destructor is responsible for cleaning up any resources
   * that the Ymodem class may have allocated during its lifetime.
   * Currently, it does not perform any specific actions.
   */
  ~Ymodem(){};

  /**
   * @brief Receives data and writes it to the provided file.
   *
   * @param ffd Reference to the file where the received data will be written.
   * @param maxsize Maximum size of the data to be received.
   * @param getname Pointer to a character array where the name of the received file will be stored.
   * @return int Status code indicating the result of the receive operation.
   * Error code:
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
  int receive(fs::File& ffd, unsigned int maxsize, char* getname);

  /**
   * @brief Transmits a file using the Ymodem protocol.
   *
   * @param sendFileName The name of the file to be transmitted.
   * @param sizeFile The size of the file to be transmitted.
   * @param ffd A reference to the file object to be transmitted.
   * @return int Status code of the transmission (0 for success, non-zero for errors).
   * Error code:
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
  int transmit(char* sendFileName, unsigned int sizeFile, fs::File& ffd);

  /**
   * @brief Sets the pin number for the LED.
   *
   * This function configures the specified pin to be used for controlling an LED.
   *
   * @param pin The pin number to be set for the LED.
   */
  void setLedPin(int pin);

  /**
   * @brief Retrieves the pin number associated with the LED.
   *
   * @return int The pin number of the LED.
   */
  int getLedPin();

private:
  int  ledPin = YMODEM_LED_ACT; /**< Pin number associated with the LED. */
  void finalizeSession();
};

#endif // YMODEMCORE_H