#include "fileSystem.h"

struct Mica
{
  time_t MeasureTime = time(NULL);
  float  Temperatura = 0;
  float  Humedad     = 0;
  float  CO2         = 0;
  float  CH2O        = 0;
  float  TVOC        = 0;
  float  PM1_0       = 0;
  float  PM2_5       = 0;
  float  PM4_0       = 0;
  float  PM10        = 0;
  float  NOX         = 0;
  float  CO          = 0;
  int    NO2         = 0;
  int    O3          = 0;
};

void setup()
{
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000));

  Serial.println("Setup");
  log_v("Mica size: %d", sizeof(Mica));

  FileSystem fs;
  Mica       mica;

  // write file until there is no space left
  int numMeasures = 0;
  while (fs.writeToFile("/test.bin", (const uint8_t*)&mica, sizeof(Mica)) == LITTLEFS_OK) {
    mica.Temperatura++;
    numMeasures++;
    Serial.printf("Measures stored = %i\r", numMeasures);
  }
  Serial.println();

  fs.printStoredFiles();
  log_v("Free space: %d", fs.getRemainingSpace());

  // read all measures
  numMeasures = 0;
  while (fs.readFromFile("/test.bin", (uint8_t*)&mica, sizeof(Mica), sizeof(Mica) * numMeasures) == LITTLEFS_OK) {
    Serial.printf("Read temperature: %f\r", mica.Temperatura);
    numMeasures++;
  }
  Serial.println();

  // delete all files
  fs.deleteAllFiles();

  // print stored files
  fs.printStoredFiles();
}

void loop()
{
  vTaskDelete(NULL);
}