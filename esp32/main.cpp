#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

const char *ssid = "replace me with your wifi name";                       //  your network SSID (name)
const char *password = "replace me with your password";  // your network password

// Define connection pins:
#define pirPin 18

// Create variables:
int val = LOW;
bool motionState = false;  // We start with no motion detected.
float h = 0;
float t = 0;

#define DHTPIN 23
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillisTemp = 0;  // last time update
unsigned long previousMillisMove = 0;  // last time update
long interval = 1000 * 60;             // interval at which to do something (milliseconds)

HTTPClient http;
WiFiClientSecure httpsClient;

void ensureWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid, password);

    delay(5000);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Setup started!");

  pinMode(pirPin, INPUT);
  dht.begin();

  WiFi.mode(WIFI_STA);  //เชื่อมต่อ Wifi
  WiFi.begin(ssid, password);

  Serial.println("\nConnecting to WiFi");
  ensureWiFi();

  httpsClient.setInsecure();
  httpsClient.setTimeout(5000);

  http.addHeader("x-api-key", "this is use to prevent spaming from malicious actors");
  http.addHeader("Content-Type", "application/json");

  Serial.println("Setup finished!");
}

void loop() {
  detectMotion();
  measureTemperature();
}

void measureTemperature() {
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(1000);
    return;
  }

  float newH = dht.readHumidity();
  float newT = dht.readTemperature();

  if (h != newH || t != newT) {
    h = newH;
    t = newT;

    Serial.print(F(" Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.println(F("°C "));

    unsigned long currentMillis = millis();

    if (currentMillis - previousMillisTemp < interval) {
      previousMillisTemp = currentMillis;
      return;
    }

    ensureWiFi();

    Serial.println("Posting to officeClimate");
    http.begin(httpsClient, "https://asia-southeast1-office-sensors-27e21.cloudfunctions.net/officeClimate");

    String temperature = "{\"temperature\":" + (String)t + ",";
    String payload = temperature + "\"humidity\":" + (String)h + "}";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode < 200 || httpResponseCode >= 400) {
      Serial.println('Failed to post to climate');
    }
  }
}

void detectMotion() {
  val = digitalRead(pirPin);

  // If motion is detected (pirPin = HIGH), do the following:
  if (val == HIGH && motionState == false) {
    // Change the motion state to true (motion detected):
    Serial.println("Motion detected!");
    motionState = true;

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisMove > interval) {
      previousMillisMove = currentMillis;
      ensureWiFi();

      http.begin(httpsClient, "https://asia-southeast1-office-sensors-27e21.cloudfunctions.net/officeMovement");

      Serial.println("Posting to officeMovement");

      int httpResponseCode = http.POST("{}");
      if (httpResponseCode < 200 || httpResponseCode >= 400) {
        Serial.println('Failed to post to movement');
      }
    }

  }
  // If no motion is detected (pirPin = LOW), do the following:
  else {
    if (motionState == true) {
      // Change the motion state to false (no motion):
      Serial.println("Motion ended!");
      motionState = false;

      delay(1000);
    }
  }
}