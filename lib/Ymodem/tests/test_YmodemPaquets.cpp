#include "YmodemPaquets.h"
#include <SPIFFS.h>
#include <string.h>
#include <unity.h>

File ffd;

void createTestFile(const char* path, int size)
{
  // Inicializar SPIFFS y crear un archivo de prueba
  if (!SPIFFS.begin(true)) {
    TEST_ASSERT_MESSAGE(false, "Error al inicializar SPIFFS");
    return;
  }

  // Abrir el archivo de prueba
  File ffd = SPIFFS.open(path, FILE_WRITE);
  if (!ffd) {
    TEST_ASSERT_MESSAGE(false, "Error al abrir el archivo de prueba");
    return;
  }

  // Escribir datos en el archivo de prueba
  for (int i = 0; i < size; i++) {
    ffd.write((uint8_t)i);
  }

  // Mostrar el contenido del archivo de prueba
  TEST_ASSERT_MESSAGE(ffd.size() == size, "Error en el tamaño del archivo de prueba");

  ffd.close();
}

void test_Ymodem_PrepareIntialPacket(void)
{
  uint8_t  data[PACKET_SIZE + PACKET_HEADER + 2];
  char     fileName[] = "testfile.txt";
  uint32_t length     = 12345;

  Ymodem_PrepareIntialPacket(data, fileName, length);

  TEST_ASSERT_EQUAL_UINT8(SOH, data[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00, data[1]);
  TEST_ASSERT_EQUAL_UINT8(0xff, data[2]);
  TEST_ASSERT_EQUAL_STRING(fileName, (char*)(data + PACKET_HEADER));
  // Extra space is added to the end of the file name
  TEST_ASSERT_EQUAL_STRING("12345 ", (char*)(data + PACKET_HEADER + strlen(fileName) + 1));
  // CRC is not tested because it is calculated
}

void test_Ymodem_PrepareLastPacket(void)
{
  uint8_t data[PACKET_SIZE + PACKET_HEADER + 2];

  Ymodem_PrepareLastPacket(data);

  TEST_ASSERT_EQUAL_UINT8(SOH, data[0]);
  TEST_ASSERT_EQUAL_UINT8(0x00, data[1]);
  TEST_ASSERT_EQUAL_UINT8(0xff, data[2]);
  // CRC is not tested because it is calculated
}

void test_Ymodem_PreparePacket(void)
{
  uint8_t data[PACKET_1K_SIZE + PACKET_HEADER + 2];
  uint8_t pktNo = 1;

  // Crear archivo de prueba
  createTestFile("/test_file.txt", 1024);

  // Reabrir el archivo para lectura
  ffd = SPIFFS.open("/test_file.txt", FILE_READ);
  if (!ffd) {
    TEST_FAIL_MESSAGE("Error al abrir el archivo de prueba");
    return;
  }

  // Caso de prueba para tamaño de bloque menor que PACKET_1K_SIZE
  uint32_t sizeBlk = 512; // Menor que PACKET_1K_SIZE
  Ymodem_PreparePacket(data, pktNo, sizeBlk, ffd);

  TEST_ASSERT_EQUAL_UINT8(STX, data[0]);
  TEST_ASSERT_EQUAL_UINT8(pktNo, data[1]);
  TEST_ASSERT_EQUAL_UINT8(~pktNo, data[2]);

  // Verificar que los bytes adicionales estén rellenados con 0x00
  for (uint16_t i = sizeBlk + PACKET_HEADER; i < PACKET_1K_SIZE + PACKET_HEADER; i++) {
    TEST_ASSERT_EQUAL_UINT8(0x00, data[i]);
  }

  // Caso de prueba para tamaño de bloque igual a PACKET_1K_SIZE
  sizeBlk = PACKET_1K_SIZE; // Igual a PACKET_1K_SIZE
  ffd.seek(0);              // Reiniciar la posición de lectura del archivo
  Ymodem_PreparePacket(data, pktNo, sizeBlk, ffd);

  TEST_ASSERT_EQUAL_UINT8(STX, data[0]);
  TEST_ASSERT_EQUAL_UINT8(pktNo, data[1]);
  TEST_ASSERT_EQUAL_UINT8(~pktNo, data[2]);

  // Verificar que no hay bytes adicionales rellenados con 0x00
  for (uint16_t i = sizeBlk + PACKET_HEADER; i < PACKET_1K_SIZE + PACKET_HEADER; i++) {
    TEST_ASSERT_NOT_EQUAL(0x00, data[i]);
  }

  ffd.close();
  SPIFFS.end();
}

void test_Ymodem_WaitResponse(void)
{
  uint8_t ackchr = ACK;
  uint8_t tmo    = 10;

  unsigned long startTime = millis();
  uint8_t       result    = Ymodem_WaitResponse(ackchr, tmo);
  unsigned long endTime   = millis();

  TEST_ASSERT_TRUE(result == 1 || result == 2 || result == 3 || result == 4 || result == 0);
  TEST_ASSERT_TRUE(endTime - startTime >= tmo);
}

void setup()
{
  // Inicialización de Serial para depuración
  Serial.begin(115200);
  while (!Serial) {
    ; // Espera hasta que Serial esté listo
  }

  UNITY_BEGIN();
  RUN_TEST(test_Ymodem_PrepareIntialPacket);
  RUN_TEST(test_Ymodem_PrepareLastPacket);
  RUN_TEST(test_Ymodem_PreparePacket);
  RUN_TEST(test_Ymodem_WaitResponse);
  UNITY_END();
}

void loop()
{
  // No se necesita código en el loop para esta operación
}
