// Includes

// I2C
#include <Wire.h>

// DHT
#include <DHT.h>

// VEML6075
#include "Adafruit_VEML6075.h"

// LiquidCrystal display
#include <LiquidCrystal_I2C.h>

// Defines

// DHT
#define DHTPIN 13
#define DHTTYPE DHT22

// Initialize

// DHT
DHT dht(DHTPIN, DHTTYPE);

// VEML6075
Adafruit_VEML6075 uv = Adafruit_VEML6075();

// LiquidCrystal Display
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Variables

// DHT variables
float t = 0.0;    // Temperature
float h = 0.0;    // Humidity

// VEML6075 variables
float uvA = 0.0;  // UVA
float uvB = 0.0;  // UVB
float uvI = 0.0;  // UV Index

// Hall variables
int hallPin = 12;
bool hall = 0;

void setup() {

  // Start Serial (for debugging purpose)
  Serial.begin(9600);
  Serial.println("Starting setup!");

  // Start I2C readings
  Wire.begin();

  // Starting sensors
  dht.begin();
  uv.begin();

  // Determine Hal Pinmode
  pinMode(hallPin, INPUT); 

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Write on lcd on startup
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.setCursor(0,1);
  lcd.print("Temperature: ");
  lcd.print(t);
  lcd.setCursor(0,2);
  lcd.print("UV Index: "); 
  lcd.print(uvI);
  lcd.setCursor(0,3);
  lcd.print("Wind speed: ");
  lcd.print(hall); // Replace with wind speed variable!!!

}

void loop() {
  // Pause between measurements
  delay(5000);

  // Take readings from sensors
  t = dht.readTemperature();
  h = dht.readHumidity();

  uvA = uv.readUVA();
  uvB = uv.readUVB();
  uvI = uv.readUVI();

  hall = digitalRead(hallPin);

  // Check if empty or failed reading
  // If not, print temperature
  if ( isnan(t) ) {
    Serial.println("Failed to read temperature!");
  } else { 
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println("°C");
  };

  // Check if empty or failed reading	
  // If not, print humidity
  if ( isnan(h) ) {
    Serial.println("Failed to read humidity!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println("%");
  };

  // Check if empty or failed reading
  // If not, print UV A
  if ( isnan(uvA ) ) {
    Serial.println("Failed to read UVA!");
  } else { 
    Serial.print("UV A: ");
    Serial.println(uvA);
  };

  // Check if empty or failed reading
  // If not, print UV B
  if ( isnan(uvB) ) {
    Serial.println("Failed to read UVB!");
  } else { 
    Serial.print("UV B: ");
    Serial.println(uvB);
  };

  // Check if empty or failed reading
  // If not, print UV Index
  if ( isnan(uvI) ) {
    Serial.println("Failed to read UV Index!");
  } else { 
    Serial.print("UV Index: ");
    Serial.println(uvI);
  };

  // Check if empty or failed reading
  // If not, print Hall data
  if ( isnan(hall) ) {
    Serial.println("Failed to read Hall sensor!");
  } else { 
    Serial.print("Hall sensor data: ");
    Serial.println(hall);
  }

  // Clear LCD
  lcd.clear();

  // Rewrite on LCD
  lcd.setCursor(0,0);
  lcd.print("Humidity: ");
  lcd.print(h);
  lcd.setCursor(0,1);
  lcd.print("Temperature: ");
  lcd.print(t);
  lcd.setCursor(0,2);
  lcd.print("UV Index: "); 
  lcd.print(uvI);
  lcd.setCursor(0,3);
  lcd.print("Wind speed: ");
  lcd.print(hall); // Replace with wind speed variable!!!

}
