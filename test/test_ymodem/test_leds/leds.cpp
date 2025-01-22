#include "ymodem.h"
#include <Arduino.h>
#include <unity.h>

Ymodem ymodem;

void test_setLedPin()
{
  int testPin = GPIO_NUM_2;
  ymodem.setLedPin(testPin);
  TEST_ASSERT_EQUAL(testPin, ymodem.getLedPin());
}

void setup()
{
  // NOTE!!! Wait for >2 secs if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);
  UNITY_BEGIN(); // Start unit testing
  RUN_TEST(test_setLedPin);
  UNITY_END(); // End unit testing
}

void loop()
{
  // Empty loop
}

void setUp(void)
{
  // Código de configuración antes de cada prueba
}

void tearDown(void)
{
  // Código de limpieza después de cada prueba
}