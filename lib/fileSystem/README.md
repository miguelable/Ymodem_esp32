# FileSystem Class Documentation

## Description

The `FileSystem` class is a wrapper for the LittleFS library that provides a simple interface for writing and reading files from the file system. It also provides methods for deleting files and displaying stored files.

## Author

- Miguel Ferrer "@MiguelFerrerF"

## Version

- 0.1

## Date

- 13 de febrero de 2025

## Methods

### Constructor

```cpp
FileSystem();
```

This constructor attempts to mount the LittleFS file system. If the initial mount fails, it logs a warning message and attempts to format the file system. After formatting, it attempts to mount the file system again. If the second mount attempt also fails, it logs an error message and exits the constructor.

### Destructor

```cpp
~FileSystem();
```

This destructor is responsible for properly ending the LittleFS file system. It ensures that any resources allocated by LittleFS are released when the `FileSystem` object is destroyed.

### writeToFile

```cpp
error_code_littefs writeToFile(const char* filename, const uint8_t* data, size_t size);
```

This method opens a file in append mode and writes the provided data to it. It ensures there is enough space available before writing and handles errors that may occur during file operations.

- **Parameters**:
  - `filename`: The name of the file to write to.
  - `data`: A pointer to the data to write.
  - `size`: The number of bytes to write to the file.
- **Returns**: `LITTLEFS_OK` on success, or an appropriate error code on failure:
  - `ERROR_OPENNING_FILE`: Error opening the file.
  - `ERROR_NO_ENOUGH_SPACE`: Not enough space available in the file system.
  - `ERROR_WRITING_FILE`: Error writing to the file.

### readFromFile

```cpp
error_code_littefs readFromFile(const char* filename, uint8_t* data, size_t size);
```

This method opens a file in read mode, seeks to the specified offset, and reads the specified amount of data into the provided buffer. It handles errors that may occur during file operations.

- **Parameters**:
  - `filename`: The name of the file to read from.
  - `data`: A pointer to the buffer where the data will be stored.
  - `size`: The number of bytes to read from the file.
  - `offset`: The offset from the beginning of the file to start reading.
- **Returns**: `LITTLEFS_OK` on success, or an appropriate error code on failure:
  - `ERROR_OPENNING_FILE`: Error opening the file.
  - `ERROR_NO_MORE_DATA`: The offset is greater than the file size or seeking to the offset failed.
  - `ERROR_READING_FILE`: Failed to read the specified amount of data from the file.

### deleteFile

```cpp
error_code_littefs deleteFile(const char* filename);
```

This method attempts to delete a file specified by the given filename from the LittleFS file system. It first opens the root directory and then tries to remove the file.

- **Parameters**:
  - `filename`: The name of the file to delete.
- **Returns**: `LITTLEFS_OK` on success, or an appropriate error code on failure:
  - `ERROR_OPENNING_DIR`: Error opening the root directory.
  - `ERROR_DELETING_FILE`: Error deleting the file.

### deleteAllFiles

```cpp
error_code_littefs deleteAllFiles();
```

This method opens the root directory of the LittleFS file system and iterates through all files, deleting each one. If the directory cannot be opened, an error code is returned. If any file cannot be deleted, an error code is returned. If no files are found, a debug message is logged.

- **Returns**: `LITTLEFS_OK` on success, or an appropriate error code on failure:
  - `ERROR_OPENNING_DIR`: Error opening the root directory.
  - `ERROR_DELETING_FILE`: Error deleting a file.

### printStoredFiles

```cpp
void printStoredFiles();
```

This method opens the root directory of the LittleFS file system and iterates through all the files stored in it. For each file, it logs the file name and size. If the directory cannot be opened, an error message is logged. If no files are found, a message indicating that no files are stored is logged.

### getRemainingSpace

```cpp
size_t getRemainingSpace();
```

This method calculates the remaining space available in the file system by subtracting the used bytes from the total bytes.

- **Returns**: The amount of remaining space in bytes.

## Error codes

The `error_code_littefs` enumeration contains the error codes that can be returned by the file system methods. The error codes are used to identify the error that occurred during the execution of the method. The possible error codes are:

- `LITTLEFS_OK`: No error occurred.
- `ERROR_OPENNING_FILE`: Error opening the file.
- `ERROR_OPENNING_DIR`: Error opening the directory.
- `ERROR_NO_ENOUGH_SPACE`: Not enough space available in the file system.
- `ERROR_WRITING_FILE`: Error writing to the file.
- `ERROR_READING_FILE`: Error reading from the file.
- `ERROR_DELETING_FILE`: Error deleting the file.
- `ERROR_NO_MORE_DATA`: No more data available in the file.

## Example Usage

The following example shows how to use the `FileSystem` class to write and read files, print stored files, and delete a file.

```cpp
#include "fileSystem.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  FileSystem fs;

  // Write data to a file
  const char* filename = "/example.txt";
  const uint8_t data[] = "Hello, LittleFS!";
  fs.writeToFile(filename, data, sizeof(data));

  // Read data from a file
  uint8_t buffer[20];
  fs.readFromFile(filename, buffer, sizeof(buffer));

  // Print stored files
  fs.printStoredFiles();

  // Delete a file
  fs.deleteFile(filename);
}

void loop() {
  // Do nothing
}
```
