/**
 * @file test_spiffs.cpp
 * @author your name (you@domain.com)
 * @brief  Test to check if the SPIFFS filesystem is working properly
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "unity.h"
#include <Arduino.h>
#include <SPIFFS.h>

#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024) // 2 MB

void test_spiffs_mount()
{
  TEST_ASSERT_TRUE(SPIFFS.begin(true));
}

void test_spiffs_format()
{
  TEST_ASSERT_TRUE(SPIFFS.format());
}

void test_spiffs_open()
{
  File file = SPIFFS.open("/test.txt", FILE_WRITE);
  TEST_ASSERT_TRUE(file);
  file.close();
}

void test_spiffs_write()
{
  File file = SPIFFS.open("/test.txt", FILE_WRITE);
  TEST_ASSERT_TRUE(file);
  file.print("Hello World!");
  file.close();
}

void test_spiffs_read()
{
  File file = SPIFFS.open("/test.txt", FILE_READ);
  TEST_ASSERT_TRUE(file);
  String content = file.readString();
  file.close();
  TEST_ASSERT_EQUAL_STRING("Hello World!", content.c_str());
}

void test_spiffs_unmount()
{
  SPIFFS.end();
}

void setup(void)
{
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_spiffs_mount);
  RUN_TEST(test_spiffs_format);
  RUN_TEST(test_spiffs_open);
  RUN_TEST(test_spiffs_write);
  RUN_TEST(test_spiffs_read);
  RUN_TEST(test_spiffs_unmount);
  UNITY_END();
}

void loop()
{
}

void setUp(void)
{
  // Código de configuración antes de cada prueba
}

void tearDown(void)
{
  // Código de limpieza después de cada prueba
}