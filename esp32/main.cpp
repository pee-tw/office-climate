#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h"
#include "pitches.h"


#define BUZZER_PIN 21
#define DOOR_SENSOR_PIN 23
#define DHTPIN 22
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
HTTPClient http;
WiFiClientSecure httpsClient;

const char *ssid = "Replace me with actual ssid";                       //  your network SSID (name)
const char *password = "Replace me with actual password";  // your network password

int timezone = 7 * 3600L; // Thailand is on GMT+7
int dst = 0;

String apiKey = "Replace me with actual api key";
String cloudFunctionUrl = "Replace me with cloud function url";
String gChatUrl = "Replace me with webhook url with tokens";

float h = 0;
float t = 0;
const int morningHour = 6;
const int eveningHour = 18;
unsigned long previousMillisTemp = 0;
unsigned long previousMillisDoor = 0;
long interval = 1000 * 60;
long alarmInterval = 1000 * 30;
long sendMsgInterval = 1000 * 60 * 5;
int prevDoorState = LOW;
int wiFiRetryThreshold = 10;
int wiFiRetryCount = 0;
struct tm timeinfo;

void ensureWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    if (wiFiRetryCount == wiFiRetryThreshold) {
      Serial.println("WiFi threshold reached try restarting");
      ESP.restart();
    }
    WiFi.disconnect();
    WiFi.begin(ssid, password);

    Serial.println(WiFi.status());
    Serial.println("Not connected retrying");
    wiFiRetryCount++;
    delay(1000 * 10);
  }
}

void startUpSound() {
  int melody[] = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
  };

  int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4
  };

  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}

void lunchMelody() {
  int currentHour = timeinfo.tm_hour;
  int currentMin = timeinfo.tm_min;

  if (currentHour != 12 || currentMin != 0) {
    return;
  }

  int melody[] = {
    NOTE_F4, NOTE_A4, NOTE_G4, NOTE_C4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_F4
  };

  int noteDurations[] = {
    4, 4, 4, 4, 4, 4, 4, 2
  };

  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }

  delay(1000 * 60);
}

void alertSound() {
  int noteDuration = 1000 / 4;
  tone(BUZZER_PIN, NOTE_G6, noteDuration);

  int pauseBetweenNotes = noteDuration * 1.30;
  delay(pauseBetweenNotes);
  noTone(BUZZER_PIN);
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Setup started!");

  WiFi.mode(WIFI_STA);  //เชื่อมต่อ Wifi
  WiFi.begin(ssid, password);

  Serial.println("\nConnecting to WiFi");

  dht.begin();

  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);  // set ESP32 pin to input pull-up mode
  pinMode(BUZZER_PIN, OUTPUT);             // set ESP32 pin to output mode
  pinMode(DHTPIN, INPUT);

  ensureWiFi();
  configTime(timezone, dst, "time1.nimt.or.th", "ntp.ku.ac.th", "itoml.live.rmutt.ac.th");

  while (!time(nullptr)) {
    Serial.print("*");
    delay(1000);
  }

  httpsClient.setInsecure();
  httpsClient.setTimeout(5000);

  http.addHeader("Content-Type", "application/json");

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println("Setup finished!");
  startUpSound();
}

void loop() {
  checkDoor();
  measureTemperature();
  lunchMelody();
}

void alertToChat() {
  ensureWiFi();

  Serial.println("Posting to chat");
  http.begin(httpsClient, gChatUrl);

  String text = "\"Office door has been opened outside of office hour exceeding alarm threshold, please check if door is properly closed\"";

  String payload = "{\"text\":" + text + "}";

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode < 200 || httpResponseCode >= 400) {
    Serial.println('Failed to post to chat');
  } else {
    Serial.println('Post to chat');
  }
}
void checkDoor() {
  int currentHour = timeinfo.tm_hour;

  // Office hour guard
  if (currentHour > morningHour && currentHour < eveningHour) {
    Serial.println('Not in office hour, skipping');
    return;
  }

  int doorState = digitalRead(DOOR_SENSOR_PIN);  // read state

  if (doorState == HIGH) {
    Serial.println("The door is open, turns on Piezo Buzzer");
    unsigned long currentMillis = millis();

    if (prevDoorState == LOW) {
      previousMillisDoor = millis();
      prevDoorState = HIGH;
    }

    if (currentMillis - previousMillisDoor < alarmInterval) {
      Serial.println("Alarm threshold not reached");
      delay(1000);
      return;
    }

    if (currentMillis - previousMillisDoor > sendMsgInterval) {
      Serial.println("Send message to chat group");
      alertToChat();
      delay(1000 * 60 * 60 * 12);
    }

    alertSound();
  } else {
    Serial.println("The door is closed, turns off Piezo Buzzer");
    digitalWrite(BUZZER_PIN, LOW);  // turn off Piezo Buzzer
    prevDoorState = LOW;
    previousMillisDoor = millis();
  }
}

void measureTemperature() {
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    ESP.restart();
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
      Serial.println("Reported within interval, skip report");
      return;
    }

    previousMillisTemp = currentMillis;

    ensureWiFi();

    Serial.println("Posting to officeClimate");

    http.addHeader("x-api-key", apiKey);
    http.begin(httpsClient, cloudFunctionUrl);

    String temperature = "{\"temperature\":" + (String)t + ",";
    String payload = temperature + "\"humidity\":" + (String)h + "}";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode < 200 || httpResponseCode >= 400) {
      Serial.println('Failed to post to climate');
    } else {
      Serial.println('Post to climate');
    }
  }
}
