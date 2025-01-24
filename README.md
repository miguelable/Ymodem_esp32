Ymodem library for ESP32 with Arduino Framework

> [!NOTE]
> Find all the code explanation [HERE](https://miguelable.github.io/Ymodem_esp32/)

### Overview

The Ymodem library provides an implementation of the Ymodem protocol for file transfer over serial communication. It is designed to work with the ESP32 microcontroller and the Arduino framework. This library allows you to send and receive files using the Ymodem protocol, which is commonly used for reliable file transfers in embedded systems.

### Features

- **File Transmission and Reception**: Supports sending and receiving files using the Ymodem protocol.
- **SPIFFS Integration**: Works seamlessly with the SPIFFS filesystem on the ESP32.
- **Serial Communication**: Utilizes the hardware serial ports of the ESP32 for data transfer.
- **Error Handling**: Includes error detection and handling mechanisms to ensure reliable file transfers.
- **Configurable**: Allows customization of various parameters such as baud rate, file size limits, and more.

### Installation

1. Download the Ymodem library and place it in the lib directory of your PlatformIO project.
2. Ensure that the platformio.ini file includes the necessary dependencies for the ESP32 and SPIFFS.

### Usage

#### Transmitting a File

To transmit a file using the Ymodem protocol, follow these steps:

1. Include the necessary headers in your source file:

```cpp
#include "YmodemCore.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
```

2. Define the SPIFFS size and maximum file size:

```cpp
#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024)        /*!< SPIFFS size in bytes */
#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000) /*!< Maximum file size */
```

3. Create an instance of the Ymodem class:

```cpp
Ymodem ymodem;
```

4. Initialize the system and configure the serial communication and SPIFFS filesystem in the setup function:

```cpp
void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for Serial to be ready
    }

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("Setup complete. Ready to send files.");
}
```

5. Implement the file transmission logic in the loop function or another appropriate function:

```cpp
void loop() {
    File file = SPIFFS.open("/example.txt", FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    int result = ymodem.transmit("example.txt", file.size(), file);
    if (result == 0) {
        Serial.println("File transmitted successfully");
    } else {
        Serial.printf("File transmission failed with error code: %d\n", result);
    }

    file.close();
    delay(10000); // Wait before sending the file again
}
```

#### Receiving a File

To receive a file using the Ymodem protocol, follow these steps:

1. Include the necessary headers in your source file:

```cpp
#include "YmodemCore.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
```

2. Define the SPIFFS size and maximum file size:

```cpp
#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024)        /*!< SPIFFS size in bytes */
#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000) /*!< Maximum file size */
```

3. Create an instance of the Ymodem class:

```cpp
Ymodem ymodem;
```

4. Initialize the system and configure the serial communication and SPIFFS filesystem in the setup function:

```cpp
void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for Serial to be ready
    }

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("Setup complete. Ready to receive files.");
}
```

5. Implement the file reception logic in the loop function or another appropriate function:

```cpp
void loop() {
    File file = SPIFFS.open("/received.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    char fileName[64];
    int result = ymodem.receive(file, MAX_FILE_SIZE, fileName);
    if (result == 0) {
        Serial.println("File received successfully");
    } else {
        Serial.printf("File reception failed with error code: %d\n", result);
    }

    file.close();
    delay(10000); // Wait before receiving the file again
}
```

## Error Codes

The Ymodem library provides the following error codes for file transmission and reception:

- -1: Abort by sender
- -2: Too many errors
- -3: Packet sequence error
- -4: Invalid file size
- -5: Filename packet error
- -6: File write error
- -7: User abort
- -8: Timeout
- -9: File size exceeds maxsize

### License

This library is licensed under the MIT License. See the LICENSE file for more information.

### Acknowledgements

This library is based on the Ymodem C implementation by [zonque](https://gist.github.com/zonque/0ae2dc8cedbcdbd9b933) and has been adapted for the ESP32 microcontroller with the Arduino framework.
