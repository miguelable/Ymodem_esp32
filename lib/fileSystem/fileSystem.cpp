/**
 * @file fileSystem.cpp
 * @author Miguel Ferrer "@MiguelFerrerF"
 * @brief Implementation of the fileSystem class
 * @version 0.1
 * @date 2025-02-13
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "fileSystem.h"

FileSystem::FileSystem()
{
  if (!LittleFS.begin()) {
    log_w("An Error has occurred while mounting LittleFS. Formatting...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      log_e("Failed to mount LittleFS after formatting");
      return;
    }
  }
}

FileSystem::~FileSystem()
{
  LittleFS.end();
}

error_code_littefs FileSystem::writeToFile(const char* filename, const uint8_t* data, size_t size)
{
  File file = LittleFS.open(filename, FILE_APPEND);
  if (!file) {
    log_e("Failed to open file for writing");
    return ERROR_OPENNING_FILE;
  }
  // Check if we have enough space to write the data
  if (size > getRemainingSpace() - 4096) {
    log_e("Not enough space to write data");
    file.close();
    return ERROR_NO_ENOUGH_SPACE;
  }
  if (file.write(data, size) != size) {
    log_e("Failed to write data to file");
    file.close();
    return ERROR_WRITING_FILE;
  }
  file.flush(); // Force write to disk
  file.close();
  return LITTLEFS_OK;
}

error_code_littefs FileSystem::readFromFile(const char* filename, uint8_t* data, size_t size, size_t offset)
{
  File file = LittleFS.open(filename, FILE_READ);
  if (!file) {
    return ERROR_OPENNING_FILE;
  }
  if (!file.seek(offset) || offset >= file.size()) {
    log_e("Offset is greater than file size");
    file.close();
    return ERROR_NO_MORE_DATA;
  }
  if (file.read(data, size) != size) {
    log_e("Failed to read data from file");
    file.close();
    return ERROR_READING_FILE;
  }
  return LITTLEFS_OK;
}

error_code_littefs FileSystem::deleteFile(const char* filename)
{
  File root = LittleFS.open("/");
  if (!root) {
    log_e("Failed to open directory");
    return ERROR_OPENNING_DIR;
  }

  if (!LittleFS.remove(filename)) {
    return ERROR_DELETING_FILE;
  }
  else {
    log_w("File deleted: %s", filename);
  }
  return LITTLEFS_OK;
}

error_code_littefs FileSystem::deleteAllFiles()
{
  File root = LittleFS.open("/");
  if (!root) {
    log_e("Failed to open directory");
    return ERROR_OPENNING_DIR;
  }

  bool hasFiles = false;
  for (File file = root.openNextFile(); file; file = root.openNextFile()) {
    hasFiles             = true;
    std::string filename = std::string("/") + file.name();
    file.close(); // Close the file before deleting it
    if (!LittleFS.remove(filename.c_str())) {
      log_e("Failed to delete file: %s", filename.c_str());
      return ERROR_DELETING_FILE;
    }
    else {
      log_w("File deleted: %s", filename.c_str());
    }
  }

  if (!hasFiles) {
    log_d("No files to delete");
  }

  root.close();
  return LITTLEFS_OK;
}

void FileSystem::printStoredFiles()
{
  File root = LittleFS.open("/");
  if (!root) {
    log_e("Failed to open directory");
    return;
  }

  bool hasFiles = false;
  for (File file = root.openNextFile(); file; file = root.openNextFile()) {
    hasFiles = true;
    log_d("File: %s, size: %d", file.name(), file.size());
    file.close();
  }

  if (!hasFiles) {
    log_d("No files stored");
  }
  root.close();
}

size_t FileSystem::getRemainingSpace()
{
  return LittleFS.totalBytes() - LittleFS.usedBytes();
}

size_t FileSystem::getFileSize(const char* filename)
{
  File file = LittleFS.open(filename, FILE_READ);
  if (!file) {
    log_e("Failed to open file for reading");
    return 0;
  }
  size_t size = file.size();
  file.close();
  return size;
}