/**
 * @file test_serial.cpp
 * @author your name (you@domain.com)
 * @brief  Test to check if the serial communication is working properly
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "unity.h"
#include <Arduino.h>

void test_serial_begin(void)
{
  Serial.begin(115200);
  TEST_ASSERT_TRUE(Serial);
}

void test_serial_print(void)
{
  Serial.begin(115200);
  TEST_ASSERT_TRUE(Serial);
  Serial.println("Hello World!");
}

void test_serial_read(void)
{
  Serial.begin(115200);
  TEST_ASSERT_TRUE(Serial);
  while (!Serial.available()) {
  }
  String content = Serial.readString();
  TEST_ASSERT_EQUAL_STRING("Hello World!\r\n", content.c_str());
}

void setup(void)
{
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_serial_begin);
  RUN_TEST(test_serial_print);
  RUN_TEST(test_serial_read);
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