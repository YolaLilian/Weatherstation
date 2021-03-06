#include <Arduino.h>

// DHT
#include <DHT.h>

#define DHTPIN 13
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

float temperatureValue = 0.0;     // Temperature
float humidityValue = 0.0;        // Humidity

// I2C
#include <Wire.h>

// VEML6075
#include "Adafruit_VEML6075.h"

Adafruit_VEML6075 uv = Adafruit_VEML6075();

float uvIValue = 0.0;             // UV Index

// Hall
const int hallPin = 12;
int rpm;                          // Rotations per minute
const unsigned long sampleTime = 10000;

float windspeedValue = 0.0;       // Wind speed

// LEDs
#include <FastLED.h>

#define NUM_LEDS 6                // Number of leds
#define DATA_PIN 14               // Data pin
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// LCD
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Signal Strength
float signalstrengthValue;        // Signal strength

// MQTT Setup
#include "Secret.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoHA.h>

WiFiClient client;
ESP8266WiFiMulti wifiMulti;
HADevice device;
HAMqtt mqtt(client, device);

const uint32_t connectTimeoutMs = 5000;

const char* ssid = WIFI_SSID_3;
const char* password = WIFI_PASSWORD_3;
// const char* ssid2 = WIFI_SSID_2;
// const char* password2 = WIFI_PASSWORD_2;
// const char* ssid3 = WIFI_SSID_3;
// const char* password3 = WIFI_PASSWORD_3;
// const char* ssid4 = WIFI_SSID_4;
// const char* password4 = WIFI_PASSWORD_4;

HASensor sensorOwner("Owner");
HASensor sensorLong("Long");
HASensor sensorLat("Lat");
HASensor sensorTemperature("Temperature");
HASensor sensorHumidity("Humidity");
HASensor sensorUV("UV");
HASensor sensorWindspeed("Wind_speed");
HASensor sensorSignalstrength("Signal_strength");

// Weather data API
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>

const char* serverName = "http://api.weatherapi.com/v1/current.json?key=456a528ddbf846b5bcb124644211510&q=51.94362193018906,4.37045934809847";                  // API address
HTTPClient http;
String payload;
String sensorReadings;
int weatherCondition = 0;

// HUE lights
String hue_server_IP = HUE_SERVER_IP;
String hue_user_ID = HUE_USER_ID;
String hueServerNameGET = "http://" + hue_server_IP + "/api/" + hue_user_ID + "/groups/5";
String hueServerNamePUT = "http://" + hue_server_IP + "/api/" + hue_user_ID + "/groups/5/action";
String response;

void setup() {

  Serial.begin(9600);
  Serial.println("Starting setup!");

  // DHT
  dht.begin();

  // I2C
  Wire.begin();

  // VEML6075
  uv.begin();

  // Hall
  pinMode(hallPin, INPUT); 

  // LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);  // GRB ordering is typical

  // FastLED.setBrightness(5);

  for (int i = 0; i < NUM_LEDS; i++) {
  // Now turn the LED off, then pause
    leds[i] = CRGB::Black;
    FastLED.show();
  }

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("--------------------");
  lcd.setCursor(5,1);
  lcd.print("Setting up");
  lcd.setCursor(3,2);
  lcd.print("weatherstation");
  lcd.setCursor(0,3);
  lcd.print("--------------------");

  // MAC address
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500); // waiting for the connection
  }
  Serial.println("Connected to the network");

}

void mqtt_start () {
    
  // HA String conversion from Secret.h
  String student_id = STUDENT_ID;
  String student_name = STUDENT_NAME;

  // Add student ID number with sensor name
  String stationNameStr = student_name + "'s Home Station";
  String ownerNameStr = student_id + " Station owner";
  String longNameStr = student_id + " Long";
  String latNameStr = student_id + " Lat";
  String temperatureNameStr = student_id + " Temperature";
  String humidityNameStr = student_id + " Humidity";
  String uvNameStr = student_id + " UV Index";
  String windspeedNameStr = student_id + " Wind Speed";
  String signalstrengthNameStr = student_id + " Signal Strength";
  
  // Convert the strings to const char*
  const char* stationName = stationNameStr.c_str();
  const char* ownerName = ownerNameStr.c_str();
  const char* longName = longNameStr.c_str();
  const char* latName = latNameStr.c_str();
  const char* temperatureName = temperatureNameStr.c_str();
  const char* humidityName = humidityNameStr.c_str();
  const char* uvName = uvNameStr.c_str();
  const char* windspeedName = windspeedNameStr.c_str();
  const char* signalstrengthName = signalstrengthNameStr.c_str();

  // Setup main device
  device.setName(stationName);
  device.setSoftwareVersion(SOFTWARE_VERSION);
  device.setManufacturer(STUDENT_NAME);
  device.setModel(MODEL_TYPE);

  // Setup owner
  sensorOwner.setName(ownerName);
  sensorOwner.setIcon("mdi:account");

  // Setup longitude and latitude
  sensorLong.setName(longName);
  sensorLong.setIcon("mdi:crosshairs-gps");
  sensorLat.setName(latName);
  sensorLat.setIcon("mdi:crosshairs-gps");

  // Setup temperature
  sensorTemperature.setName(temperatureName);
  sensorTemperature.setDeviceClass("temperature");
  sensorTemperature.setUnitOfMeasurement("??C");

  // Setup humidity
  sensorHumidity.setName(humidityName);
  sensorHumidity.setDeviceClass("humidity");
  sensorHumidity.setUnitOfMeasurement("%");

  // Setup UV sensor
  sensorUV.setName(uvName);
  sensorUV.setIcon("mdi:sun-wireless");

  // Setup wind speed
  sensorWindspeed.setName(windspeedName);
  sensorWindspeed.setUnitOfMeasurement("km/u");
  sensorWindspeed.setIcon("mdi:weather-windy");

  // Setup signal strength
  sensorSignalstrength.setName(signalstrengthName);
  sensorSignalstrength.setDeviceClass("signal_strength");
  sensorSignalstrength.setUnitOfMeasurement("dBm");

  // Start MQTT
  mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);

  while (!mqtt.isConnected()) {
      mqtt.loop();
      Serial.print(".");
      delay(500); // waiting for the connection
  }
  
  Serial.println("Connected to MQTT broker");

  // Set values
  sensorOwner.setValue(STUDENT_NAME);
  sensorLat.setValue(LAT, (uint8_t)15U);
  sensorLong.setValue(LONG, (uint8_t)15U);

}

String httpGETRequest(const char* serverName) {

  http.begin(client, serverName);
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    payload = http.getString();   // Get the request response payload
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return payload;

}

void httpPUTRequest(String hueServerNamePUT, int weatherCondition) {
  http.begin(client, hueServerNamePUT);
  http.addHeader("Content-Type", "text/plain");
  Serial.println(weatherCondition);

  if (weatherCondition == 1003 || weatherCondition == 1006 || weatherCondition == 1009 || weatherCondition == 1087 || weatherCondition == 1180 || weatherCondition == 1183 || weatherCondition == 1186 || weatherCondition == 1189 || weatherCondition == 1192 || weatherCondition == 1195 || weatherCondition == 1273 || weatherCondition == 1276|| weatherCondition == 1279 || weatherCondition == 1282) {
    int httpResponseCode = http.PUT("{\"on\": true}");
  } else if ( weatherCondition == 1000) {
    int httpResponseCode = http.PUT("{\"on\": false}");
  }
  
  http.end();

}

void rain() {
  leds[0] = CRGB( 36, 229, 250);
  leds[5] = CRGB( 36, 229, 250);
  FastLED.show();
  delay(100);
  leds[1] = CRGB( 36, 229, 250);
  leds[4] = CRGB( 36, 229, 250);
  FastLED.show();
  delay(100);
  leds[2] = CRGB( 36, 229, 250);
  leds[3] = CRGB( 36, 229, 250);
  FastLED.show();
  delay(100);

  leds[0] = CRGB::Black;
  leds[5] = CRGB::Black;
  FastLED.show();
  delay(100);
  leds[1] = CRGB::Black;
  leds[4] = CRGB::Black;
  FastLED.show();
  delay(100);
  leds[2] = CRGB::Black;
  leds[3] = CRGB::Black;
  FastLED.show();
  delay(1000);

}

void thunder() {

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB( 255, 255, 15);
    FastLED.show();
    leds[i] = CRGB::Black;
    FastLED.show();
  }

}

int getRPM() {

  int count = 0;
  boolean countFlag = LOW;
  unsigned long currentTime = 0;
  unsigned long startTime = millis();
  while (currentTime <= sampleTime) {
    ESP.wdtFeed();
    if (digitalRead(hallPin) == 1) {
      countFlag = HIGH;
    }
    if (digitalRead(hallPin) == 0 && countFlag == HIGH) {
      count++;
      countFlag=LOW;
    }
    currentTime = millis() - startTime;
  }

  int countRpm = (60000/float(sampleTime)) * count;
  int countKmU = 0.07515353 * countRpm + 0.03593882;

  if (countRpm == 0) {
    countKmU = 0;
  }

  return countKmU;

}

void loop() {

  String currentSSID = WiFi.SSID();
  Serial.println(currentSSID);

  delay(5000);

  Serial.printf("1st Connection status: %d\n", WiFi.status());
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.printf("2nd Connection status: %d\n", WiFi.status());

  temperatureValue = dht.readTemperature();
  humidityValue = dht.readHumidity();
  uvIValue = uv.readUVI();
  windspeedValue = getRPM();
  Serial.print(windspeedValue);
  Serial.println(" km/u");

  // Clear LCD
  lcd.clear();

  // Rewrite on LCD
  lcd.setCursor(0,0);
  lcd.print("Temperature: ");
  lcd.print(temperatureValue, 1);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0,1); 
  lcd.print("Humidity: ");
  lcd.print(humidityValue, 1);
  lcd.print("%");
  lcd.setCursor(0,2);
  lcd.print("UV Index: "); 
  lcd.print(uvIValue);
  lcd.setCursor(0,3);
  lcd.print("Wind speed: ");
  lcd.print(windspeedValue, 1);
  lcd.print("km/h");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500); // waiting for the connection
  }
  Serial.println();
  Serial.println("Connected to the network");

  mqtt_start();

  Serial.printf("3rd Connection status: %d\n", WiFi.status());

  // Check if empty or failed readings
  // If not, print value
  if ( isnan(temperatureValue) ) {
    Serial.println("Failed to read temperature!");
  } else if ( isnan(humidityValue) ) {
    Serial.println("Failed to read humidity!");
  } else if ( isnan(uvIValue) ) {
   Serial.println("Failed to read UV Index!");
  } else if ( isnan(windspeedValue) ) {
    Serial.println("Failed to read Hall sensor!");
  } else { 
    sensorTemperature.setValue(temperatureValue);
    sensorHumidity.setValue(humidityValue);
    sensorUV.setValue(uvIValue);
    sensorWindspeed.setValue(windspeedValue); 
  };

  // Signal strength check
  signalstrengthValue = WiFi.RSSI();
  sensorSignalstrength.setValue(signalstrengthValue);
  
  httpPUTRequest( hueServerNamePUT, weatherCondition);

  String sensorReadings = httpGETRequest(serverName);
  JSONVar myWeather = JSON.parse(sensorReadings);

  if (JSON.typeof(myWeather) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  JSONVar currentWeather = myWeather["current"]["condition"]["code"];
  // int weatherCondition = int(currentWeather); // Real weather
  weatherCondition = 1192; // Simulate weatherconditions

  // Thunder 
  if ( weatherCondition == 1087 || weatherCondition == 1273 || weatherCondition == 1276 || weatherCondition == 1279 || weatherCondition == 1282 ) {
    thunder();
    FastLED.clear();
  }
  // Light rain
  else if ( weatherCondition == 1180 || weatherCondition == 1183 ) {
    rain();
    FastLED.clear();
  }   
  // Medium rain
  else if ( weatherCondition == 1186 || weatherCondition == 1189 ) {
    rain();
    rain();
    FastLED.clear();
  } 
  // Heavy rain
  else if ( weatherCondition == 1192 || weatherCondition == 1195 ) {
    rain();
    rain();
    rain();
    FastLED.clear();
  }
    
}