#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "pitches.h"
#include <NTPClient.h>

#define BUZZER_PIN 21
#define DOOR_SENSOR_PIN 23
#define DHTPIN 22
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
HTTPClient http;
WiFiClientSecure httpsClient;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char *ssid = "Replace me with actual ssid";                       //  your network SSID (name)
const char *password = "Replace me with actual password";

String apiKey = "<Removed for obvious reasons>";
String cloudFunctionUrl = "https://asia-southeast1-office-sensors-27e21.cloudfunctions.net/officeClimate";
String gChatUrl = "<Secrets included in the url>";

const int morningHour = 6;
const int eveningHour = 18;
int lunchDay = 0;
int alarmDay = 0;
unsigned long previousMillisTemp = 0;
unsigned long previousMillisDoor = 0;
long interval = 1000 * 60;
long alarmInterval = 1000 * 30;
long sendMsgInterval = 1000 * 60 * 3;
int prevDoorState = LOW;
int wiFiRetryThreshold = 10;

int wiFiRetryCount = 0;
int currentDay = 0;
float h = 0;
float t = 0;
// struct tm timeinfo;

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
  // int currentHour = timeinfo.tm_hour;
  // int currentMin = timeinfo.tm_min;
  // int currentDay = timeinfo.tm_mday;
  int currentHour = timeClient.getHours();
  int currentMin = timeClient.getMinutes();


  if (currentHour != 12 || currentMin != 0 || currentDay == lunchDay) {
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

  lunchDay = currentDay;
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
  // configTime(timezone, dst, "pool.ntp.org");
  timeClient.begin();
  timeClient.setTimeOffset(7 * 3600);

  // while (!time(nullptr)) {
  //   Serial.print("*");
  //   delay(1000);
  // }

  httpsClient.setInsecure();
  httpsClient.setTimeout(5000);

  http.addHeader("Content-Type", "application/json");

  // if (!getLocalTime(&timeinfo)) {
  //   Serial.println("Failed to obtain time");
  //   ESP.restart();
  // }

  Serial.println("Setup finished!");
  startUpSound();
}

void loop() {
  timeSync();
  checkDoor();
  measureTemperature();
  lunchMelody();
}

void timeSync() {
  timeClient.update();
  
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  currentDay = ptm->tm_mday;
}

void alertToChat() {
  ensureWiFi();

  Serial.println("Posting to chat");
  http.begin(httpsClient, gChatUrl);

  // https://arduinojson.org/v6/example/generator/ Size calculated here
  StaticJsonDocument<147> doc;

  String json;
  doc["text"] = "Office door has been opened outside of office hour exceeding alarm threshold, please check if door is properly closed";
  serializeJson(doc, json);

  int httpResponseCode = http.POST(json);

  if (httpResponseCode < 200 || httpResponseCode >= 400) {
    Serial.println('Failed to post to chat');
  } else {
    Serial.println('Post to chat');
    delay(1000 * 60 * 60 * 12); // Pause for 12 hours
  }
}
void checkDoor() {
  int currentHour = timeClient.getHours();

  // Office hour guard
  if (currentHour > morningHour && currentHour < eveningHour) {
    Serial.println('Not in office hour, skipping');
    return;
  }
  // Message sent guard
  if (alarmDay == currentDay) {
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
      alarmDay = currentDay;
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
