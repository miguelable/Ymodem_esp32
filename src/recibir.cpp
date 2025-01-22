

#include "YmodemCore.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>
#include <Update.h>

// Define CONFIG_SPIFFS_SIZE with an appropriate value
#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024) // 2 MB

#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000)

// Instancia de la clase Ymodem
Ymodem ymodem;

void setup()
{
  // Inicialización de Serial para depuración
  Serial.begin(115200);
  while (!Serial) {
    ; // Espera hasta que Serial esté listo
  }

  // Inicialización de SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  // Configuración del pin del LED de actividad YModem
  pinMode(YMODEM_LED_ACT, OUTPUT);
  digitalWrite(YMODEM_LED_ACT, YMODEM_LED_ACT_ON ^ 1);

  // Configuración de UART (Serial1)
  Serial1.begin(115200, SERIAL_8N1, 16, 17); // RX, TX pins
  if (!Serial1) {
    Serial.println("Error al inicializar Serial1");
    return;
  }

  // Mensaje de configuración completa
  Serial.println("Setup completo. Listo para recibir archivos.");
}

void loop()
{
  static int nfile = 1;
  char       fname[128];
  char       orig_name[256];
  uint32_t   max_fsize;
  int        rec_res = -1;

  // ==== Recepción de archivo ====
  max_fsize = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  if (max_fsize > 16 * 1024) {
    if (max_fsize > MAX_FILE_SIZE) {
      max_fsize = MAX_FILE_SIZE;
    }

    sprintf(fname, "/firmware-%d.bin", nfile);
    File ffd = SPIFFS.open(fname, FILE_WRITE);
    if (ffd) {
      Serial.println("\r\nRecibiendo archivo, inicia la transferencia YModem "
                     "en el host...\r\n");
      rec_res = ymodem.receive(ffd, max_fsize, orig_name);
      ffd.close();
      Serial.println("\r\n");

      if (rec_res > 0) {
        Serial.printf("Transferencia completada. Tamaño=%d, Nombre original: \"%s\"\n", rec_res, fname);

        // ==== Cargar el binario al ESP32 ====
        File bin_file = SPIFFS.open(fname, FILE_READ);
        if (bin_file) {
          size_t bin_size = bin_file.size();
          if (Update.begin(bin_size)) {
            Serial.println("Iniciando la actualización OTA...");
            size_t written = Update.writeStream(bin_file);
            if (written == bin_size)
              Serial.println("Escritura completa.");
            else
              Serial.printf("Escritura incompleta. Escrito solo %d de %d bytes\n", written, bin_size);

            if (Update.end()) {
              if (Update.isFinished()) {
                Serial.println("Actualización OTA completada. Reiniciando...");
                ESP.restart();
              }
              else
                Serial.println("Actualización OTA no completada.");
            }
            else
              Serial.printf("Error en la actualización OTA: %s\n", Update.errorString());
          }
          else
            Serial.println("No se pudo iniciar la actualización OTA.");
          bin_file.close();
        }
        else
          Serial.printf("Error al abrir el archivo binario \"%s\" para OTA.\n", fname);
      }
      else {
        Serial.printf("Error en la transferencia. Código de error=%d\n", rec_res);
        SPIFFS.remove(fname);
      }
    }
    else
      Serial.printf("Error al abrir el archivo \"%s\" para recibir.\n", fname);

    delay(5000);
    nfile++;
  }
  else {
    Serial.printf("Sistema de archivos lleno. Espacio restante: %u bytes\n", max_fsize);
  }

  delay(10);
}