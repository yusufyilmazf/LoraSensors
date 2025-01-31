#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

#define DHTPIN 7           // DHT11 sensörünün bağlı olduğu pin
#define DHTTYPE DHT11      // Sensör tipi
#define ss 10              // LoRa çip seçme pin
#define rst 9              // LoRa reset pin
#define dio0 2             // LoRa DIO0 pin

#define SOIL_SENSOR_PIN A0 // Toprak nemi sensörünün bağlı olduğu pin
#define PIR_SENSOR_PIN 3   // PIR sensörünün bağlı olduğu pin

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // LoRa modülünü başlat
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6)) {      // 433 MHz frekansı
    Serial.println("LoRa başlatılamadı!");
    while (1);
  }

  dht.begin();
  pinMode(SOIL_SENSOR_PIN, INPUT);  // Toprak nemi sensörü pini giriş olarak ayarlandı
  pinMode(PIR_SENSOR_PIN, INPUT);   // PIR sensörü pini giriş olarak ayarlandı

  Serial.println("LoRa, DHT11, Toprak Nem ve PIR sensörleri başlatıldı");
}

void loop() {
  // DHT11 sensöründen veri okuma
  float h = dht.readHumidity();  // Nem
  float t = dht.readTemperature();  // Sıcaklık
  int soilMoisture = analogRead(SOIL_SENSOR_PIN);
  int pirState = digitalRead(PIR_SENSOR_PIN);

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT sensör verisi okunamadı");
    return;
  }

  // Toprak Nem Verisi Gönder
  LoRa.beginPacket();
  LoRa.print("Soil:" + String(soilMoisture));
  LoRa.endPacket();
  delay(500);

  // PIR Sensörü Verisi Gönder
  LoRa.beginPacket();
  LoRa.print("PIR:" + String(pirState));
  LoRa.endPacket();
  delay(500);

  // Sıcaklık ve Nem Verisi Gönder
  LoRa.beginPacket();
  LoRa.print("T:" + String(t) + "C H:" + String(h) + "%");
  LoRa.endPacket();
  delay(500);
}
