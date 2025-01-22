#include "FS.h"
#include "SPIFFS.h"
#include "ymodem.h"
#include <Arduino.h>
#include <unity.h>

// Define CONFIG_SPIFFS_SIZE with an appropriate value
#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024) // 2 MB

#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000)

Ymodem ymodem;

void setUp(void)
{
  // Set up code here, to run before each test
}

void tearDown(void)
{
  // Clean up code here, to run after each test
}

void test_file_reception(void)
{
  Serial.begin(115200);
  TEST_ASSERT_TRUE(Serial);

  // Spiffs setup
  TEST_ASSERT_TRUE(SPIFFS.begin(true));

  // Configuración del pin del LED de actividad YModem
  pinMode(YMODEM_LED_ACT, OUTPUT);
  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);

  static int nfile = 1;
  char       fname[128];
  char       orig_name[256];
  uint32_t   max_fsize;
  int        rec_res = -1;

  max_fsize = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  if (max_fsize > 16 * 1024) {
    if (max_fsize > MAX_FILE_SIZE) {
      max_fsize = MAX_FILE_SIZE;
    }

    sprintf(fname, "/yfile-%d.bin", nfile);
    File ffd = SPIFFS.open(fname, FILE_WRITE);
    if (ffd) {
      rec_res = ymodem.receive(ffd, max_fsize, orig_name);
      ffd.close();

      TEST_ASSERT_GREATER_THAN(0, rec_res);
      TEST_ASSERT_EQUAL_STRING("test.bin", orig_name);
    }
    else {
      TEST_FAIL_MESSAGE("Failed to open file for writing");
    }
    nfile++;
  }
  else {
    TEST_FAIL_MESSAGE("Not enough space in SPIFFS");
  }
}

void setup()
{
  UNITY_BEGIN();
  RUN_TEST(test_file_reception);
  UNITY_END();
}

void loop()
{
  // Empty loop
}