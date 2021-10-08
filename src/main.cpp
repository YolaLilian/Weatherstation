// Includes

// DHT
#include <DHT.h>

// Defines

// DHT
#define DHTPIN 2
#define DHTTYPE DHT22

// Initialize

// DHT
DHT dht(DHTPIN, DHTTYPE);

// Variables

// DHT variables
float t = 0.0;    // Temperature
float h = 0.0;    // Humidity

void setup() {

  // Start Serial (for debugging purpose)
  Serial.begin(9600);
  Serial.println("Starting setup!");

  // Starting sensors
  dht.begin();

}

void loop() {
  // Pause between measurements
  delay(5000);

  // Take readings from sensor
  t = dht.readTemperature();
  h = dht.readHumidity();

  // Check if empty or failed reading
  if ( isnan(t) ) {
    Serial.println("Failed to read temperature!");
  }
  if ( isnan(h) ) {
    Serial.println("Failed to read humidity!");
  }

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");

// Serial.print("Humidity: "); Serial.println(dht.readHumidity());
// Serial.print("Temperature: "); Serial.println(dht.readTemperature());

}
