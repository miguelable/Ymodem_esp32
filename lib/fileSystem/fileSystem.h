/**
 * @file fileSystem.h
 * @author Miguel Ferrer "@MiguelFerrerF"
 * @brief Implementation of a file system using LittleFS
 * @version 0.1
 * @date 2025-02-13
 *
 * This class is a wrapper for the LittleFS library, it provides a simple interface to write and read files from the file system.
 * It also provides a method to delete files and print the stored files.
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <LittleFS.h>

/**
 * @brief Error codes for the file system
 *
 * This enum contains the error codes that can be returned by the file system methods.
 * The error codes are used to identify the error that occurred during the execution of the method.
 * Possible error codes are:
 * - LITTLEFS_OK: No error occurred
 * - ERROR_OPENNING_FILE: Error opening the file
 * - ERROR_OPENNING_DIR: Error opening the directory
 * - ERROR_READING_FILE: Error reading the file
 * - ERROR_NO_ENOUGH_SPACE: Not enough space to write the file
 * - ERROR_WRITING_FILE: Error writing the file
 * - ERROR_DELETING_FILE: Error deleting the file
 * - ERROR_NO_MORE_DATA: No more data to read from the file
 *
 */
enum error_code_littefs : uint8_t
{
  LITTLEFS_OK,
  ERROR_OPENNING_FILE,
  ERROR_OPENNING_DIR,
  ERROR_READING_FILE,
  ERROR_NO_ENOUGH_SPACE,
  ERROR_WRITING_FILE,
  ERROR_DELETING_FILE,
  ERROR_NO_MORE_DATA
};

/**
 * @brief  File system class
 *
 * @details This class is a wrapper for the LittleFS library, it provides a simple interface to write and read
 * files from the file system. It also provides a method to delete files and print the stored files.
 *
 */
class FileSystem
{
public:
  /**
   * @brief Constructor for the FileSystem class.
   *
   * This constructor attempts to mount the LittleFS file system. If the initial
   * mount fails, it logs a warning message and attempts to format the file system.
   * After formatting, it attempts to mount the file system again. If the second
   * mount attempt also fails, it logs an error message and exits the constructor.
   */
  FileSystem();

  /**
   * @brief Destructor for the FileSystem class.
   *
   * This destructor is responsible for properly ending the LittleFS filesystem.
   * It ensures that any resources allocated by LittleFS are released when the
   * FileSystem object is destroyed.
   */
  ~FileSystem();

  /**
   * @brief Writes data to a file in the LittleFS filesystem.
   *
   * This function opens a file in append mode and writes the provided data to it.
   * It ensures there is enough space available before writing and handles errors
   * that may occur during the file operations.
   *
   * @param filename The name of the file to write to.
   * @param data A pointer to the data to be written.
   * @param size The size of the data to be written.
   * @return error_code_littefs Returns LITTLEFS_OK on success, or an appropriate error code on failure:
   *         - ERROR_OPENNING_FILE: Failed to open the file for writing.
   *         - ERROR_NO_ENOUGH_SPACE: Not enough space to write the data.
   *         - ERROR_WRITING_FILE: Failed to write data to the file.
   */
  error_code_littefs writeToFile(const char* filename, const uint8_t* data, size_t size);


  /**
   * @brief Writes data to a file in the filesystem.
   * 
   * This function writes the specified data to a file with the given filename
   * in the filesystem. If the file does not exist, it will be created. If the
   * file already exists, its contents will be overwritten.
   * 
   * @param filename The name of the file to write to. This should include the
   *                 full path if necessary.
   * @param data A pointer to the data to be written to the file.
   * @param size The size of the data to be written, in bytes.
   * @return error_code_littefs Returns an error code indicating the success or
   *                            failure of the operation.
   */
  error_code_littefs writeToFile(const char* filename, const char* data, size_t size);

  /**
   * @brief Reads data from a file in the LittleFS filesystem.
   *
   * This function opens a file in read mode, seeks to the specified offset, and reads the specified amount of data into the provided buffer.
   *
   * @param filename The name of the file to read from.
   * @param data A pointer to the buffer where the read data will be stored.
   * @param size The number of bytes to read from the file.
   * @param offset The position in the file from where to start reading.
   * @return error_code_littefs Returns LITTLEFS_OK on success, or an appropriate error code on failure:
   *         - ERROR_OPENNING_FILE: Failed to open the file.
   *         - ERROR_NO_MORE_DATA: The offset is greater than the file size or seeking to the offset failed.
   *         - ERROR_READING_FILE: Failed to read the specified amount of data from the file.
   */
  error_code_littefs readFromFile(const char* filename, uint8_t* data, size_t size, size_t offset = 0);

  /**
   * @brief Deletes a file from the LittleFS filesystem.
   *
   * This function attempts to delete a file specified by the given filename
   * from the LittleFS filesystem. It first opens the root directory and then
   * tries to remove the file.
   *
   * @param filename The name of the file to be deleted.
   * @return error_code_littefs Returns LITTLEFS_OK if the file was successfully deleted,
   *         ERROR_OPENNING_DIR if the root directory could not be opened,
   *         or ERROR_DELETING_FILE if the file could not be deleted.
   */
  error_code_littefs deleteFile(const char* filename);

  /**
   * @brief Deletes all files in the LittleFS filesystem.
   *
   * This function opens the root directory of the LittleFS filesystem and iterates
   * through all files, deleting each one. If the directory cannot be opened, an
   * error code is returned. If any file cannot be deleted, an error code is returned.
   * If no files are found, a debug message is logged.
   *
   * @return error_code_littefs Returns LITTLEFS_OK if all files are successfully deleted,
   *                            ERROR_OPENNING_DIR if the root directory cannot be opened,
   *                            or ERROR_DELETING_FILE if a file cannot be deleted.
   */
  error_code_littefs deleteAllFiles();

  /**
   * @brief Prints the names and sizes of all files stored in the LittleFS filesystem.
   *
   * This function opens the root directory of the LittleFS filesystem and iterates
   * through all the files stored in it. For each file, it logs the file name and size.
   * If the directory cannot be opened, an error message is logged. If no files are found,
   * a message indicating that no files are stored is logged.
   */
  void printStoredFiles();

  /**
   * @brief Get the remaining space in the file system.
   *
   * This function calculates the remaining space available in the file system
   * by subtracting the used bytes from the total bytes.
   *
   * @return size_t The amount of remaining space in bytes.
   */
  size_t getRemainingSpace();

  /**
   * @brief Retrieves the size of a file in bytes.
   *
   * This function opens the specified file in read mode, determines its size,
   * and then closes the file. If the file cannot be opened, an error is logged
   * and the function returns 0.
   *
   * @param filename The path to the file whose size is to be determined.
   * @return The size of the file in bytes, or 0 if the file could not be opened.
   */
  size_t getFileSize(const char* filename);
};

#endif // FILESYSTEM_H