// Includes
// I2C
#include <Wire.h>

// DHT
#include <DHT.h>

// VEML6075
#include "Adafruit_VEML6075.h"

// LiquidCrystal display
#include <LiquidCrystal_I2C.h>

// MQTT Setup
#include "Secret.h"
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>

// LEDs
#include <FastLED.h>

// Initialize
// Initialize WiFi
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);

// Defines
// DHT
#define DHTPIN 13
#define DHTTYPE DHT22

// HA sensors and/or devices
HASensor sensorOwner("Owner");
HASensor sensorLong("Long");
HASensor sensorLat("Lat");
HASensor sensorTemperature("Temperature");
HASensor sensorHumidity("Humidity");
HASensor sensorSignalstrength("Signal_strength");

// LEDs
#define NUM_LEDS 6
#define DATA_PIN 14
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// Initialize
// DHT
DHT dht(DHTPIN, DHTTYPE);

// VEML6075
Adafruit_VEML6075 uv = Adafruit_VEML6075();

// LiquidCrystal Display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Variables
// DHT variables
float temperatureValue = 0.0;    // Temperature
float humidityValue = 0.0;    // Humidity

// VEML6075 variables
float uvAValue = 0.0;  // UVA
float uvBValue = 0.0;  // UVB
float uvIValue = 0.0;  // UV Index

// Hall variables
int hallPin = 12;
bool hallValue = 0;

// Signal Strength variable
float signalstrengthValue;

// // Frequency variables
// unsigned long lastReadAt = millis();
// unsigned long lastTemperatureSend = millis();
// bool lastInputState = false;

void setup() {

  // Start Serial (for debugging purpose)
  Serial.begin(9600);
  Serial.println("Starting setup!");

  // Start I2C readings
  Wire.begin();

  // Starting sensors
  dht.begin();
  uv.begin();

  // Determine Hall Pinmode
  pinMode(hallPin, INPUT); 

  // Set up LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);  // GRB ordering is typical

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Write on lcd on startup
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
  String signalstrengthNameStr = student_id + " Signal Strength";
  
  // Convert the strings to const char*
  const char* stationName = stationNameStr.c_str();
  const char* ownerName = ownerNameStr.c_str();
  const char* longName = longNameStr.c_str();
  const char* latName = latNameStr.c_str();
  const char* temperatureName = temperatureNameStr.c_str();
  const char* humidityName = humidityNameStr.c_str();
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

  // Setup signal strength
  sensorSignalstrength.setName(signalstrengthName);
  sensorSignalstrength.setDeviceClass("signal_strength");
  sensorSignalstrength.setUnitOfMeasurement("dBm");

  // Setup temperature
  sensorTemperature.setName(temperatureName);
  sensorTemperature.setDeviceClass("temperature");
  sensorTemperature.setUnitOfMeasurement("°C");

  // Setup humidity
  sensorHumidity.setName(humidityName);
  sensorHumidity.setDeviceClass("humidity");
  sensorHumidity.setUnitOfMeasurement("%");

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

void loop() {
  // Pause between measurements
  delay(3000);

  // MQTT
  mqtt.loop();

  // Take readings from sensors
  temperatureValue = dht.readTemperature();
  humidityValue = dht.readHumidity();

  uvAValue = uv.readUVA();
  uvBValue = uv.readUVB();
  uvIValue = uv.readUVI();

  hallValue = digitalRead(hallPin);

  // Signal strength check
  signalstrengthValue = WiFi.RSSI();
  Serial.println(signalstrengthValue);
  sensorSignalstrength.setValue(signalstrengthValue);

  // Check if empty or failed reading
  // If not, print temperature
  if ( isnan(temperatureValue) ) {
    Serial.println("Failed to read temperature!");
  } else { 
    Serial.print("Temperature: ");
    Serial.print(temperatureValue);
    Serial.println("°C");
    sensorTemperature.setValue(temperatureValue);
  };

  // Check if empty or failed reading	
  // If not, print humidity
  if ( isnan(humidityValue) ) {
    Serial.println("Failed to read humidity!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humidityValue);
    Serial.println("%");
    sensorHumidity.setValue(humidityValue);
  };

  // Check if empty or failed reading
  // If not, print UV A
  if ( isnan(uvAValue) ) {
    Serial.println("Failed to read UVA!");
  } else { 
    Serial.print("UV A: ");
    Serial.println(uvAValue);
  };

  // Check if empty or failed reading
  // If not, print UV B
  if ( isnan(uvBValue) ) {
    Serial.println("Failed to read UVB!");
  } else { 
    Serial.print("UV B: ");
    Serial.println(uvBValue);
  };

  // Check if empty or failed reading
  // If not, print UV Index
  if ( isnan(uvIValue) ) {
    Serial.println("Failed to read UV Index!");
  } else { 
    Serial.print("UV Index: ");
    Serial.println(uvIValue);
  };

  // Check if empty or failed reading
  // If not, print Hall data
  if ( isnan(hallValue) ) {
    Serial.println("Failed to read Hall sensor!");
  } else { 
    Serial.print("Hall sensor data: ");
    Serial.println(hallValue);
  }

  // Loop LEDs
  for (int i = 0; i < 5; i++) {
    leds[i] = CRGB( 36, 229, 250);
    FastLED.show();
  }
  delay(3000);
  for (int i = 0; i < 5; i++) {
  // Now turn the LED off, then pause
    leds[i] = CRGB::Black;
    FastLED.show();
  }
  delay(3000);

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

}
