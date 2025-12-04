#include <Wifi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

const char* ssid = "SPC";
const char* password = "javelin1669";

#define API_KEY "AIzaSyCq-nrjFTORo72Q6CD93i7w4N13ml04Tho";
#define DATABASE_URL "https://post-test-iot-default-rtdb.asia-southeast1.firebasedatabase.app/";
#define USER_EMAIL "wawantod@gmail.com";
#define USER_PASS "wawantod3089";

#define dht 23
#define ldr 19
#define soil 18

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("==== SMART PLANT GREENHOUSE ====");
  Serial.println("Inisialisasi sistem...\n");

  // Pin modes
  pinMode(LDR_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(OBJECT_PIN, INPUT);

  // Connect WiFi
  connectWifi();

  // Setup NTP Time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sinkronisasi waktu dengan NTP...");
  delay(2000);

  // Firebase config
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;

  Serial.println("Menghubungkan ke Firebase...");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWifi(true);

  unsigned long fbStart = millis();
  while (!Firebase.ready() && millis() - fbStart < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (Firebase.ready()) {
    Serial.println("\n Firebase terhubung!");
    Serial.println(" Sistem siap monitoring!\n");
  } else {
    Serial.println("\n Firebase gagal terhubung, sistem tetap berjalan...\n");
  }
}

void loop() {
  // Cek koneksi WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus! Mencoba reconnect...");
    connectWifi();
  }

  // Update sensor secara berkala
  unsigned long now = millis();
  if (now - lastSensorUpdate > sensorInterval) {
    lastSensorUpdate = now;
    bacaDanKirimData();
  }
}

// Fungsi koneksi WiFi
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Menghubungkan ke WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (millis() - start > 20000) {
      Serial.println("\n Gagal terhubung Wifi - restart...");
      ESP.restart();
    }
  }
  Serial.println("\n WiFi Terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


// Fungsi untuk mendapatkan timestamp epoch dalam milliseconds
unsigned long getTimestamp() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Gagal mendapat waktu NTP, gunakan millis()");
    return millis();
  }
  time(&now);
  return ((unsigned long)now) * 1000;  // Convert ke milliseconds untuk JavaScript
}


// Fungsi untuk membaca sensor dan kirim ke Firebase
void bacaDanKirimData() {
  Serial.println("\n==================================");
  Serial.println("| PEMBACAAN SENSOR GREENHOUSE    |");
  Serial.println("==================================");

  // === BACA LDR (Cahaya)
  int rawLdr = analogRead(LDR_PIN);
  // Mapping: LDR semakin gelap = nilai ADC semakin tinggi
  // Invert untuk mendapat persentase cahaya (0% = gelap, 100% = terang)
  int lightLevel = map(rawLdr, 4095, 0, 0, 100);
  lightLevel = constrain(lightLevel, 0, 100);

  Serial.printf("Cahaya: %d %% (ADC=%d)\n", lightLevel, rawLdr);

  // === BACA SOIL MOISTURE
  int rawSoil = analogRead(SOIL_PIN);
  // Mapping: Sensor kering = nilai tinggi, basah = nilai rendah
  // Invert untuk mendapat persentase kelembaban (0% = kering, 100% = basah)
  int soilPercent = map(rawSoil, 4095, 0, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  Serial.printf("Kelembaban Tanah: %d %% (ADC=%d)\n", soilPercent, rawSoil);
  if (soilPercent < 40) {
    Serial.println("  STATUS: KERING - Perlu penyiraman!");
  } else {
    Serial.println("STATUS: Kelembaban cukup");
  }

  // === BACA SENSOR DIGITAL
  bool motionDetected = digitalRead(PIR_PIN) == HIGH;
  bool flameDetected = digitalRead(FLAME_PIN) == HIGH;
  bool objectDetected = digitalRead(OBJECT_PIN) == HIGH;

  Serial.printf("Gerakan (PIR): %s\n", motionDetected ? "TERDETEKSI " : "Tidak ada");
  Serial.printf("Api: %s\n", flameDetected ? "TERDETEKSI " : "Aman");
  Serial.printf("Objek: %s\n", objectDetected ? "TERDETEKSI" : "Tidak ada");

  // === KIRIM KE FIREBASE
  if (Firebase.ready()) {
    Serial.println("\n\\ Mengirim data ke Firebase...");

    String basePath = "/greenhouse/sensors";
    bool allSuccess = true;

    // Kirim Light Level
    if (Firebase.RTDB.setInt(&fbdo, basePath + "/lightlevel", lightLevel)) {
      Serial.println("lightLevel terkirim");
    } else {
      Serial.printf("lightLevel gagal: %s\n", fbdo.errorReason().c_str());
      allSuccess = false;
    }

    // Kirim Soil Moisture
    if (Firebase.RTDB.setInt(&fbdo, basePath + "/soilMoisture", soilPercent)) {
      Serial.println("soilMoisture terkirim");
    } else {
      Serial.printf("soilMoisture gagal: %s\n", fbdo.errorReason().c_str());
      allSuccess = false;
    }

    // Kirim Motion (PIR)
    if (Firebase.RTDB.setBool(&fbdo, basePath + "/motion", motionDetected)) {
      Serial.println("motion terkirim");
    } else {
      Serial.printf("motion gagal: %s\n", fbdo.errorReason().c_str());
      allSuccess = false;
    }

    // Kirim Flame
    if (Firebase.RTDB.setBool(&fbdo, basePath + "/flame", flameDetected)) {
      Serial.println("flame terkirim");
    } else {
      Serial.printf("flame gagal: %s\n", fbdo.errorReason().c_str());
      allSuccess = false;
    }

    // Kirim Object
    if (Firebase.RTDB.setBool(&fbdo, basePath + "/object", objectDetected)) {
      Serial.println("object terkirim");
    } else {
      Serial.printf("object gagal: %s\n", fbdo.errorReason().c_str());
      allSuccess = false;
    }

    // Kirim Timestamp (epoch milliseconds untuk JavaScript Date)
    unsigned long timestamp = getTimestamp();
    if (Firebase.RTDB.setDouble(&fbdo, basePath + "/timestamp", timestamp)) {
      Serial.printf("timestamp terkirim (%lu)\n", timestamp);
    } else {
      Serial.printf("timestamp gagal: %s\n", fbdo.errorReason().c_str());
      allSuccess = false;
    }

    if (allSuccess) {
      Serial.println("\n Semua data berhasil dikirim!");
    } else {
      Serial.println("\n Beberapa data gagal dikirim");
    }

  } else {
    Serial.println("\n Firebase belum siap, skip pengiriman");
  }

  Serial.println("------------------------------------");

  // Delay kecil untuk stabilitas
  delay(100);
}
