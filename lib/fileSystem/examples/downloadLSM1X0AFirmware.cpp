#include "HTTPClient.h"
#include "WiFi.h"
#include "filesystem.h"

const char* ssid     = "inBiot_devices";                // your network SSID (name)
const char* password = "inBiot_IAQ";                    // your network password
const char* host     = "ota.plankton.myinbiotdemo.com"; // your OTA server host

void saveLSM1X0AfwAWS(const char* moduleType);

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  saveLSM1X0AfwAWS("LSM100A");
}

void loop()
{
}

void saveLSM1X0AfwAWS(const char* moduleType)
{
  HTTPClient  HTTP;
  char        request[110];
  std::string binaryFile = "/" + std::string(moduleType) + "_SDK_V104_240129.bin";
  sprintf(request, "http://%s/%s%s", host, moduleType, binaryFile.c_str());
  HTTP.begin(request);
  HTTP.GET();
  WiFiClient client  = HTTP.getStream();
  int        totlen  = HTTP.getSize();
  int        currlen = 0;
  FileSystem SPIFFS;
  SPIFFS.deleteAllFiles();
  while (currlen < totlen) {
    int len = client.available();
    if (len > 0) {
      char buf[1024];
      int  read = client.readBytes(buf, sizeof(buf));
      currlen += read;
      if (SPIFFS.writeToFile(binaryFile.c_str(), buf, read) != LITTLEFS_OK) {
        Serial.print("[LORA] ");
        Serial.println("Error writing to SPIFFS");
        return;
      }
      Serial.printf("Downloading %s: %d%%\r", binaryFile.c_str(), (currlen * 100) / totlen);
      Serial.printf("\r");
    }
    if (currlen >= totlen) {
      log_i("Download complete: %s\n", binaryFile.c_str());
      break;
    }
  }
  Serial.printf("File size: %d bytes\n", SPIFFS.getFileSize(binaryFile.c_str()));
  Serial.printf("File system free space: %d bytes\n", SPIFFS.getRemainingSpace());
  SPIFFS.printStoredFiles(); // Print all files in the file system
}