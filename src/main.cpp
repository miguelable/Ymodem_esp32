#include "ymodem.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPIFFS.h>

// Define CONFIG_SPIFFS_SIZE with an appropriate value
#define CONFIG_SPIFFS_SIZE (2 * 1024 * 1024) // 2 MB

#define MAX_FILE_SIZE (CONFIG_SPIFFS_SIZE - 0x2000)

// Instancia de la clase Ymodem
Ymodem ymodem;

void createFile();

void setup() {
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

void loop() {
  static int nfile = 1;
  char fname[128];
  char orig_name[256];
  char send_name[128];
  uint32_t max_fsize;
  int rec_res = -1, trans_res = -1;

  // ==== Recepción de archivo ====
  max_fsize = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  if (max_fsize > 16 * 1024) {
    if (max_fsize > MAX_FILE_SIZE) {
      max_fsize = MAX_FILE_SIZE;
    }

    sprintf(fname, "/yfile-%d.bin", nfile);
    File ffd = SPIFFS.open(fname, FILE_WRITE);
    if (ffd) {
      Serial.println("\r\nRecibiendo archivo, inicia la transferencia YModem "
                     "en el host...\r\n");
      rec_res = ymodem.receive(ffd, max_fsize, orig_name);
      ffd.close();
      Serial.println("\r\n");

      if (rec_res > 0) {
        Serial.printf(
            "Transferencia completada. Tamaño=%d, Nombre original: \"%s\"\n",
            rec_res, fname);
      } else {
        Serial.printf("Error en la transferencia. Código de error=%d\n",
                      rec_res);
        SPIFFS.remove(fname);
      }
    } else {
      Serial.printf("Error al abrir el archivo \"%s\" para recibir.\n", fname);
    }

    delay(5000);

    // ==== Envío del archivo de vuelta ====
    if (rec_res > 0) {
      sprintf(send_name, "yfile-%d.bin", nfile);
      ffd = SPIFFS.open(fname, FILE_READ);
      if (ffd) {
        Serial.printf("\r\nEnviando archivo \"%s\", inicia la recepción YModem "
                      "en el host...\r\n",
                      fname);
        trans_res = ymodem.transmit(send_name, rec_res, ffd);
        ffd.close();
        Serial.println("\r\n");

        if (trans_res == 0) {
          Serial.println("Transferencia completada exitosamente.");
        } else {
          Serial.printf("Error en la transferencia. Código de error=%d\n",
                        trans_res);
        }
      } else {
        Serial.printf("Error al abrir el archivo \"%s\" para enviar.\n", fname);
      }
    }

    nfile++;
  } else {
    Serial.printf("Sistema de archivos lleno. Espacio restante: %u bytes\n",
                  max_fsize);
  }

  delay(30000);
}
