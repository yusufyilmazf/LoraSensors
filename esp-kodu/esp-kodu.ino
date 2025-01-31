#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>

#define ss 5              // LoRa çip seçme pin
#define rst 14            // LoRa reset pin
#define dio0 2            // LoRa DIO0 pin

// WiFi Bilgileri
const char* ssid = "Sabzora";
const char* password = "Ane1r!2518";

// Web sunucusu oluşturuluyor
AsyncWebServer server(80);

// Sensör verileri
String soilMoisture = "No data received yet";
String pirStatus = "No data received yet";
String tempHumidityData = "No data received yet";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 başlatılıyor...");

  // SPIFFS başlatma
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS başlatılamadı!");
    return;
  }
  Serial.println("SPIFFS başarıyla başlatıldı!");

  // WiFi'ye bağlan
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi bağlanıyor...");
  }
  WiFi.setSleep(false);  // WiFi uyku modunu kapat, bağlantı stabilitesi için.
  Serial.println("WiFi Bağlantısı Başarılı!");
  Serial.print("IP adresi: ");
  Serial.println(WiFi.localIP());

  // LoRa başlatma
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {  
    Serial.println("LoRa başlatılamadı!");
    while (1);
  }
  Serial.println("LoRa Başlatıldı!");

  // **SPIFFS'ten index.html dosyasını sunma**
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // JSON formatında sensör verilerini döndürme
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String jsonResponse = "{\"soilMoisture\":\"" + soilMoisture + "\",\"pirStatus\":\"" + pirStatus + "\",\"tempHumidityData\":\"" + tempHumidityData + "\"}";
    request->send(200, "application/json", jsonResponse);
  });

  // Sunucuyu başlat
  server.begin();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    Serial.println("Received: " + received);

    if (received.startsWith("Soil:")) {
      soilMoisture = received.substring(5);
    } else if (received.startsWith("PIR:")) {
      pirStatus = received.substring(4);
    } else if (received.startsWith("T:")) {
      tempHumidityData = received;
    }
  }

  yield();  // Watchdog resetlenmesini önlemek için eklenmeli
  vTaskDelay(50 / portTICK_PERIOD_MS);  // FreeRTOS için gecikme
}
