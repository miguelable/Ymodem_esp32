/**
 * @file YmodemCore.cpp
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

#include "YmodemCore.h"

Ymodem::Ymodem()
{
  Ymodem(YMODEM_RX_PIN, YMODEM_TX_PIN);
}

Ymodem::Ymodem(int rxPin, int txPin)
{
  ledPin = YMODEM_LED_PIN;
  Ymodem_Config(rxPin, txPin);
}

void Ymodem::Ymodem_Config(int rxPin, int txPin)
{
  uart_config_t uart_config = {
    .baud_rate  = 115200,
    .data_bits  = UART_DATA_8_BITS,
    .parity     = UART_PARITY_DISABLE,
    .stop_bits  = UART_STOP_BITS_1,
    .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
  };
  uart_param_config(EX_UART_NUM, &uart_config);
  uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
  setYmodemPins(rxPin, txPin);
}

void Ymodem::setLedPin(int pin)
{
  ledPin             = pin;
  gpio_config_t conf = {
    .pin_bit_mask = (1ULL << ledPin),      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    .mode         = GPIO_MODE_OUTPUT,      /*!< GPIO mode */
    .pull_up_en   = GPIO_PULLUP_DISABLE,   /*!< Disable pull-up resistors */
    .pull_down_en = GPIO_PULLDOWN_DISABLE, /*!< Disable pull-down resistors */
    .intr_type    = GPIO_INTR_DISABLE      /*!< Disable interrupts */
  };
  gpio_config(&conf);

  std::string doneMsg = std::string("LED pin set to ") + std::to_string(ledPin) + "\n";
  uart_write_bytes(UART_NUM_0, doneMsg.c_str(), doneMsg.length());
}

int Ymodem::getLedPin()
{
  return ledPin;
}

void Ymodem::setYmodemPins(int rxPin, int txPin)
{
  uart_set_pin(EX_UART_NUM, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_set_baudrate(EX_UART_NUM, 115200);
}

#ifdef YMODEM_LSM1X0A
void configureGpioPin(int pin)
{
  gpio_config_t conf = {
    .pin_bit_mask = (1ULL << pin),         /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    .mode         = GPIO_MODE_OUTPUT,      /*!< GPIO mode */
    .pull_up_en   = GPIO_PULLUP_DISABLE,   /*!< Disable pull-up resistors */
    .pull_down_en = GPIO_PULLDOWN_DISABLE, /*!< Disable pull-down resistors */
    .intr_type    = GPIO_INTR_DISABLE      /*!< Disable interrupts */
  };
  gpio_config(&conf);
}

void performResetCycle(int pin, int delayMs)
{
  gpio_set_level((gpio_num_t)pin, 0);
  vTaskDelay(pdMS_TO_TICKS(delayMs));

  gpio_set_level((gpio_num_t)pin, 1);
  vTaskDelay(pdMS_TO_TICKS(delayMs));
}

void sendResetCommand()
{
  uart_write_bytes(EX_UART_NUM, "1", 1);
}

void waitForModuleResponse(int timeoutMs)
{
  unsigned long startTime = millis();
  while (millis() - startTime < timeoutMs) {
    char response[2] = {0};
    int  bytesRead   = uart_read_bytes(EX_UART_NUM, (uint8_t*)response, sizeof(response) - 1, 100 / portTICK_RATE_MS);
    if (bytesRead > 0 && response[0] == 'C') {
      break; // Exit loop if 'C' is received
    }
    uart_write_bytes(UART_NUM_0, response, bytesRead); // Send response to UART_NUM_0 for debugging
  }
}

void Ymodem::resetExternalModule(int resetPin)
{
  constexpr int timeoutMs    = 10000; // Timeout in milliseconds
  constexpr int resetDelayMs = 10;    // Delay in milliseconds

  configureGpioPin(resetPin);                // Configure the GPIO pin as output
  performResetCycle(resetPin, resetDelayMs); // Perform reset cycle
  sendResetCommand();                        // Send reset command to the module
  waitForModuleResponse(timeoutMs);          // Wait for response from the module
}
#endif

/**
 * @brief End the Ymodem session.
 *
 * This function is called to perform any necessary cleanup or finalization
 * tasks at the end of a Ymodem session. If YMODEM_LED_ACT is defined, it will
 * toggle the state of the LED connected to the specified pin.
 *
 * @note The LED state is toggled by XORing the current state with 1.
 */
void Ymodem::endYmodemSession()
{
#if YMODEM_LED_ACT
  gpio_set_level((gpio_num_t)YMODEM_LED_PIN, YMODEM_LED_ACT_ON ^ 1)
#endif
}

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

  endYmodemSession();
  return size;
}

int Ymodem::transmit(const char* sendFileName)
{
  int        err;
  FileSystem fs;

  unsigned int sizeFile = fs.getFileSize(sendFileName);
  if (sizeFile == 0) {
    return -5; // Filename packet error
  }

  // Correct the file name if it starts with '/'
  char* fileName = (char*)sendFileName;
  if (fileName[0] == '/') {
    fileName++;
  }

  // Wait for response from receiver
  err = waitForReceiverResponse();
  if (err != 0) {
    return err;
  }

  // Send initial packet
  err = sendInitialPacket(fileName, sizeFile);
  if (err != 0) {
    return err;
  }

  // Send file blocks
  err = sendFileBlocks(sendFileName, fs);
  if (err != 0) {
    return err;
  }

  // Send EOT
  err = sendEOT();
  if (err != 0) {
    return err;
  }

  // Send last packet
  err = sendLastPacket();
  if (err != 0) {
    return err;
  }

  return 0; // file transmitted successfully
}