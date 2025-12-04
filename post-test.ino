#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

const char* WIFI_SSID = "SPC";
const char* WIFI_PASSWORD = "javelin1669"; 

#define API_KEY "AIzaSyCq-nrjFTORo72Q6CD93i7w4N13ml04Tho"
#define DATABASE_URL "https://post-test-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "wawantod@gmail.com"
#define USER_PASS "wawantod3089" 

#define DHT_PIN 23
#define LDR_PIN 19
#define SOIL_PIN 18

#define PIR_PIN 5
#define FLAME_PIN 4
#define OBJECT_PIN 21

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long lastSensorUpdate = 0;
const long sensorInterval = 2000; 

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

void connectWifi();

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("==== SMART PLANT GREENHOUSE ====");

  pinMode(LDR_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(OBJECT_PIN, INPUT);
  
  analogReadResolution(12);

  connectWifi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASS;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
     connectWifi();
  }

  if (millis() - lastSensorUpdate > sensorInterval) {
    lastSensorUpdate = millis();
    bacaDanKirimData();
  }
}

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());
}

unsigned long getTimestamp() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return millis();
  time(&now);
  return ((unsigned long)now) * 1000;
}

void bacaDanKirimData() {
  Serial.println("\n--- Membaca Sensor ---");

  int rawLdr = analogRead(LDR_PIN);
  int rawSoil = analogRead(SOIL_PIN);

  int lightLevel = map(rawLdr, 4095, 0, 0, 100);
  lightLevel = constrain(lightLevel, 0, 100);

  int soilPercent = map(rawSoil, 4095, 0, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  bool motionDetected = digitalRead(PIR_PIN);
  bool flameDetected = digitalRead(FLAME_PIN);
  bool objectDetected = digitalRead(OBJECT_PIN);

  Serial.printf("Soil: %d%% | Light: %d%% | Motion: %d\n", soilPercent, lightLevel, motionDetected);

  if (Firebase.ready()) {
    String basePath = "/greenhouse/sensors";
    
    FirebaseJson json;
    json.set("soilMoisture", soilPercent);
    json.set("lightlevel", lightLevel);
    json.set("motion", motionDetected);
    json.set("flame", flameDetected);
    json.set("object", objectDetected);
    json.set("timestamp", getTimestamp());

    if (Firebase.RTDB.updateNode(&fbdo, basePath, &json)) {
       Serial.println("✓ Data Terkirim!");
    } else {
       Serial.printf("✕ Gagal: %s\n", fbdo.errorReason().c_str());
    }
  }
}