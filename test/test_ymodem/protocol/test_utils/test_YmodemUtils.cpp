#include "YmodemUtils.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <unity.h>

void uart_simulator_task(void* pvParameters)
{
  // Simular un mensaje de YMODEM
  const char* test_data = "C1234567890 ";
  while (1) {
    for (int i = 0; i < strlen(test_data); i++) {
      uart_write_bytes(EX_UART_NUM, &test_data[i], 1);
      vTaskDelay(100 / portTICK_PERIOD_MS); // Simular un retardo entre bytes
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Simular un retardo entre transmisiones
  }
}

void test_crc16(void)
{
  const unsigned char test_data[]  = "123456789";
  unsigned short      expected_crc = 0x31C3; // Precomputed CRC16 for "123456789"
  unsigned short      result_crc   = crc16(test_data, sizeof(test_data) - 1);
  TEST_ASSERT_EQUAL_HEX16(expected_crc, result_crc);
}

void test_Send_Byte(void)
{
  char     test_char = 'A';
  uint32_t result    = Send_Byte(test_char);
  TEST_ASSERT_EQUAL_UINT32(0, result);
}

void test_Receive_Byte(void)
{
  unsigned char received_char;
  int32_t       result = Receive_Byte(&received_char, 1000);
  TEST_ASSERT_TRUE(result == 0 || result == -1);
}

void test_send_CA(void)
{
  send_CA();
  // Add assertions or checks if possible
}

void test_send_ACK(void)
{
  send_ACK();
  // Add assertions or checks if possible
}

void test_send_ACKCRC16(void)
{
  send_ACKCRC16();
  // Add assertions or checks if possible
}

void test_send_NAK(void)
{
  send_NAK();
  // Add assertions or checks if possible
}

void test_send_CRC16(void)
{
  send_CRC16();
  // Add assertions or checks if possible
}

void test_Receive_Packet(void)
{
  uint8_t data[PACKET_1K_SIZE + PACKET_OVERHEAD];
  int     length;
  int32_t result = Receive_Packet(data, &length, 1000);
  TEST_ASSERT_TRUE(result == 0 || result == -1 || result == -2);
}

void setup()
{
  // Inicialización de Serial para depuración
  Serial.begin(115200);
  while (!Serial) {
    ; // Espera hasta que Serial esté listo
  }

  // Crear la tarea simuladora de UART
  // xTaskCreate(uart_simulator_task, "uart_simulator_task", 2048, NULL, 5, NULL);

  UNITY_BEGIN();
  RUN_TEST(test_crc16);
  RUN_TEST(test_Send_Byte);
  RUN_TEST(test_Receive_Byte);
  RUN_TEST(test_send_CA);
  RUN_TEST(test_send_ACK);
  RUN_TEST(test_send_ACKCRC16);
  RUN_TEST(test_send_NAK);
  RUN_TEST(test_send_CRC16);
  RUN_TEST(test_Receive_Packet);
  UNITY_END();
}

void loop()
{
  // Ejecución de pruebas unitarias
}