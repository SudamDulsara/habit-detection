#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define DHTPIN   4
#define DHTTYPE  DHT22
#define PIRPIN   13
#define ACSPIN   34

const char* WIFI_SSID   = "WiFiName";
const char* WIFI_PASS   = "WiFiPassword";
const char* SERVER_IP   = "IP";
const char* SERVER_PORT = "3000";

String DHT_URL;
String PIR_URL;
String ACS_URL;

//DHT22 STATE
DHT dht(DHTPIN, DHTTYPE);
int   dhtFailCount    = 0;
float lastGoodTemp    = -1;
float lastGoodHum     = -1;

//PIR STATE
int  lastReportedState  = -1;
unsigned long lastMotionTime = 0;
const unsigned long GRACE_PERIOD = 30000;

//ACS712 STATE
const int   ACS_SAMPLES  = 1000;
const float MAINS_VOLTS  = 230.0;   // change to 120.0 if on US mains
float       acsMidpoint  = 0;
bool        lastFanOn    = false;

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
  analogSetAttenuation(ADC_11db);

  // ACS712 calibration - must happen with fan OFF
  Serial.println("=================================");
  Serial.println("CALIBRATING ACS712... keep fan OFF");
  Serial.println("=================================");
  float total = 0;
  for (int i = 0; i < 5000; i++) {
    total += analogRead(ACSPIN);
    delayMicroseconds(200);
  }
  acsMidpoint = (total / 5000.0) * (3.3 / 4095.0);
  Serial.print("Calibrated midpoint: ");
  Serial.print(acsMidpoint, 4);
  Serial.println("V");

  connectWiFi();

  // Build endpoint URLs once
  DHT_URL = String("http://") + SERVER_IP + ":" + SERVER_PORT + "/api/dht22";
  PIR_URL = String("http://") + SERVER_IP + ":" + SERVER_PORT + "/api/pir";
  ACS_URL = String("http://") + SERVER_IP + ":" + SERVER_PORT + "/api/acs712";

  Serial.println("All sensors working.");
  Serial.println("   DHT22  → " + DHT_URL);
  Serial.println("   PIR    → " + PIR_URL);
  Serial.println("   ACS712 → " + ACS_URL);
}

void loop() {

  //PIR - every 500ms
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

  //DHT22 - every 15 seconds
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

  //ACS712 - every 30 seconds
  {
    static unsigned long lastACS = 0;
    if (millis() - lastACS >= 30000) {
      lastACS = millis();

      float sumSquares = 0;
      for (int i = 0; i < ACS_SAMPLES; i++) {
        float raw  = analogRead(ACSPIN) * (3.3 / 4095.0);
        float diff = raw - acsMidpoint;
        sumSquares += diff * diff;
        delayMicroseconds(100);
      }

      float rmsVoltage = sqrt(sumSquares / ACS_SAMPLES);
      float current    = rmsVoltage / 0.185;
      if (current < 0.55) current = 0.00;    // noise filter
      float wattage    = current * MAINS_VOLTS;
      bool  fanOn      = current > 0.10;
      String applianceState = fanOn ? "ON" : "OFF";

      Serial.println("\n─── ACS712 ────────────────────────────");
      Serial.println("  Current   : " + String(current, 3) + " A");
      Serial.println("  Power     : " + String(wattage, 1) + " W");
      Serial.println("  Fan       : " + applianceState);

      String json = String("{\"voltage\":") + String(rmsVoltage, 4) +
                    ",\"current\":" + String(current, 3) +
                    ",\"wattage\":" + String(wattage, 1) +
                    ",\"fanOn\":"   + (fanOn ? "true" : "false") +
                    ",\"applianceState\":\"" + applianceState + "\"}";
      postData(ACS_URL, json);

      lastFanOn = fanOn;
    }
  }
}
