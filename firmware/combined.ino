#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define DHTPIN   4
#define DHTTYPE  DHT22
#define PIRPIN   13

const char* WIFI_SSID   = "WiFiName";
const char* WIFI_PASS   = "WiFiPassword";
const char* SERVER_IP   = "IP";
const char* SERVER_PORT = "3000";

String DHT_URL;
String PIR_URL;

//DHT22 STATE
DHT dht(DHTPIN, DHTTYPE);
int   dhtFailCount    = 0;
float lastGoodTemp    = -1;
float lastGoodHum     = -1;

//PIR STATE
int  lastReportedState  = -1;
unsigned long lastMotionTime = 0;
const unsigned long GRACE_PERIOD = 30000;

//WIFI CONNECTION
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
}

void postData(const String& url, const String& jsonBody) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost - reconnecting...");
    connectWiFi();
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(jsonBody);

  if (code == 201)
    Serial.println("Saved!");
  else if (code > 0)
    Serial.println("Server responded: " + String(code));
  else
    Serial.println("Could not reach server (code " + String(code) + ")");

  http.end();
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PIRPIN, INPUT);

  connectWiFi();

  // Build endpoint URLs once
  DHT_URL = String("http://") + SERVER_IP + ":" + SERVER_PORT + "/api/dht22";
  PIR_URL = String("http://") + SERVER_IP + ":" + SERVER_PORT + "/api/pir";

  Serial.println("All sensors working.");
  Serial.println("   DHT22 → " + DHT_URL);
  Serial.println("   PIR   → " + PIR_URL);
}

void loop() {

  //PIR - every 5s
  {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= 500) {
      lastCheck = millis();

      int reading = digitalRead(PIRPIN);
      unsigned long now = millis();

      if (reading == 1) lastMotionTime = now;   // Reset grace period timer

      int stableState;
      if (reading == 1) {
        stableState = 1;                          // Motion → Occupied immediately
      } else if (now - lastMotionTime > GRACE_PERIOD) {
        stableState = 0;                          // No motion for 30 s → Unoccupied
      } else {
        stableState = lastReportedState;          // Still in grace period — hold state
      }

      if (stableState != lastReportedState && stableState != -1) {
        String occupancy = (stableState == 1) ? "Occupied" : "Unoccupied";

        Serial.println("\n─── PIR ───────────────────────────────");
        Serial.println("  Motion   : " + String(reading));
        Serial.println("  Occupancy: " + occupancy);
        if (stableState == 0) Serial.println("  (confirmed after 30 s of no motion)");

        String json = "{\"motion\":"      + String(stableState) +
                      ",\"occupancy\":\"" + occupancy + "\"}";
        postData(PIR_URL, json);

        lastReportedState = stableState;
      }
    }
  }

  //DHT22 - every 15seconds
  {
    static unsigned long lastDHT = 0;
    if (millis() - lastDHT >= 15000) {
      lastDHT = millis();

      float temperature = dht.readTemperature();
      float humidity    = dht.readHumidity();

      if (isnan(temperature) || isnan(humidity)) {
        dhtFailCount++;
        Serial.println("DHT22 read failed! Attempt #" + String(dhtFailCount));
        if (dhtFailCount >= 3)
          Serial.println("3 consecutive failures — check wiring on pin " + String(DHTPIN));
        return;
      }

      if (temperature < -40 || temperature > 80 || humidity < 0 || humidity > 100) {
        Serial.println("⚠️  DHT22 value out of valid range — discarding.");
        return;
      }

      dhtFailCount = 0;
      lastGoodTemp = temperature;
      lastGoodHum  = humidity;

      // Comfort classification
      String comfortStatus;
      if (temperature >= 20 && temperature <= 26 && humidity >= 40 && humidity <= 60)
        comfortStatus = "Comfortable";
      else if (temperature > 26 && temperature <= 30)
        comfortStatus = "Warm - AC/Fan justified";
      else if (temperature < 20)
        comfortStatus = "Overcooling risk";
      else
        comfortStatus = "Check conditions";

      Serial.println("\n─── DHT22 ─────────────────────────────");
      Serial.println("  Temperature : " + String(temperature) + " °C");
      Serial.println("  Humidity    : " + String(humidity)    + " %");
      Serial.println("  Comfort     : " + comfortStatus);

      String json = "{\"temperature\":"    + String(temperature) +
                    ",\"humidity\":"       + String(humidity)    +
                    ",\"comfortStatus\":\"" + comfortStatus + "\"}";
      postData(DHT_URL, json);
    }
  }
}
