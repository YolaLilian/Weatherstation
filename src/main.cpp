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
int hallPin = 12;
bool hallValue = 0;               // Hall sensor true or false

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
#include <ArduinoHA.h>

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);

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

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.print(humidityValue);
  lcd.setCursor(0,1);
  lcd.print("Temperature: ");
  lcd.print(temperatureValue);
  lcd.setCursor(0,2);
  lcd.print("UV Index: "); 
  lcd.print(uvIValue);
  lcd.setCursor(0,3);
  lcd.print("Wind speed: ");
  lcd.print(hallValue); // Replace with wind speed variable!!!

  // MAC address
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500); // waiting for the connection
  }
  Serial.println();
  Serial.println("Connected to the network");

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
  sensorTemperature.setUnitOfMeasurement("°C");

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

void loop() {

  delay(3000);

  temperatureValue = dht.readTemperature();
  humidityValue = dht.readHumidity();
  uvIValue = uv.readUVI();
  hallValue = digitalRead(hallPin);

  // Check if empty or failed reading
  // If not, print temperature
  if ( isnan(temperatureValue) ) {
    Serial.println("Failed to read temperature!");
  } else { 
    sensorTemperature.setValue(temperatureValue);
  };

  // Check if empty or failed reading	
  // If not, print humidity
  if ( isnan(humidityValue) ) {
    Serial.println("Failed to read humidity!");
  } else {
    sensorHumidity.setValue(humidityValue);
  };

  // Check if empty or failed reading
  // If not, print UV Index
  if ( isnan(uvIValue) ) {
    Serial.println("Failed to read UV Index!");
  } else { 
    sensorUV.setValue(uvIValue);
  };

  // Check if empty or failed reading
  // If not, print Hall data
  if ( isnan(hallValue) ) {
    Serial.println("Failed to read Hall sensor!");
  } else { 
    sensorWindspeed.setValue(hallValue); 
  }

   // Clear LCD
  lcd.clear();

  // Rewrite on LCD
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.print(humidityValue);
  lcd.setCursor(0,1);
  lcd.print("Temperature: ");
  lcd.print(temperatureValue);
  lcd.setCursor(0,2);
  lcd.print("UV Index: "); 
  lcd.print(uvIValue);
  lcd.setCursor(0,3);
  lcd.print("Wind speed: ");
  lcd.print(hallValue); // Replace with wind speed variable!!!

  // Signal strength check
  signalstrengthValue = WiFi.RSSI();
  // Serial.println(signalstrengthValue);
  sensorSignalstrength.setValue(signalstrengthValue);

  String sensorReadings = httpGETRequest(serverName);
  JSONVar myWeather = JSON.parse(sensorReadings);

  if (JSON.typeof(myWeather) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  JSONVar currentWeather = myWeather["current"]["condition"]["code"];
  // int weatherCondition = int(currentWeather);
  int weatherCondition = 1087;

  if ( weatherCondition == 1009) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB( 36, 229, 250);
      FastLED.show();
    }
  } else if ( weatherCondition == 1087 ) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB( 255, 255, 15);
      FastLED.show();
    }
  }

}