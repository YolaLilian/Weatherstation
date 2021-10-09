// Includes

// I2C
#include <Wire.h>

// DHT
#include <DHT.h>

// VEML6075
#include "Adafruit_VEML6075.h"

// Defines

// DHT
#define DHTPIN 2
#define DHTTYPE DHT22

// Initialize

// DHT
DHT dht(DHTPIN, DHTTYPE);

// VEML6075
Adafruit_VEML6075 uv = Adafruit_VEML6075();

// Variables

// DHT variables
float t = 0.0;    // Temperature
float h = 0.0;    // Humidity

// VEML6075 variables
float uvA = 0.0;  // UVA
float uvB = 0.0;  // UVB
float uvI = 0.0;  // UV Index

void setup() {

  // Start Serial (for debugging purpose)
  Serial.begin(9600);
  Serial.println("Starting setup!");

  // Start I2C readings
  Wire.begin();

  // Starting sensors
  dht.begin();
  uv.begin();

}

void loop() {
  // Pause between measurements
  delay(5000);

  // Take readings from sensor
  t = dht.readTemperature();
  h = dht.readHumidity();

  uvA = uv.readUVA();
  uvB = uv.readUVB();
  uvI = uv.readUVI();

  // Check if empty or failed reading
  if ( isnan(t) ) {
    Serial.println("Failed to read temperature!");
  } 
  // If not, print temperature
  else { 
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println("°C");
  };

  // Check if empty or failed reading	
  if ( isnan(h) ) {
    Serial.println("Failed to read humidity!");
  } 
  // If not, print humidity
  else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println("%");
  };

  if ( isnan(uvA ) ) {
    Serial.println("Failed to read UVA!");
  } 
  // If not, print UV A
  else { 
    Serial.print("UV A: ");
    Serial.println(uvA);
  };

  if ( isnan(uvB) ) {
    Serial.println("Failed to read UVB!");
  } 
  // If not, print UV B
  else { 
    Serial.print("UV B: ");
    Serial.println(uvB);
  };

  if ( isnan(uvI) ) {
    Serial.println("Failed to read UV Index!");
  } 
  // If not, print UV Index
  else { 
    Serial.print("UV Index: ");
    Serial.println(uvI);
  };

}
