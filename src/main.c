#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Pin Definitions ---
#define ONE_WIRE_BUS 4        // DS18B20 Data pin
#define CURRENT_PIN 34        // Potentiometer pin (Simulated Current)
#define ALERT_LED_PIN 2       // Alert LED pin

// --- Thresholds for Alerts ---
const float TEMP_THRESHOLD = 75.0;     // Celsius
const float CURRENT_THRESHOLD = 15.0;  // Simulated Amps
const float VIBE_THRESHOLD = 15.0;     // m/s^2 (Total acceleration magnitude)

// --- Sensor Objects ---
Adafruit_MPU6050 mpu;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  pinMode(ALERT_LED_PIN, OUTPUT);
  
  Serial.println("Initializing Motor Monitoring System...");

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip!");
    while (1) { delay(10); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  
  // Initialize DS18B20
  sensors.begin();
  
  Serial.println("System Ready. Monitoring Started.");
  Serial.println("-----------------------------------");
}

void loop() {
  bool faultDetected = false;

  // 1. Read Temperature
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // 2. Read Simulated Current (Map 0-4095 ADC to 0-20 Amps)
  int rawCurrent = analogRead(CURRENT_PIN);
  float simulatedAmps = (rawCurrent / 4095.0) * 20.0; 

  // 3. Read Vibration (Acceleration)
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  // Calculate total acceleration magnitude (vector sum)
  float totalVibration = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));

  // --- Print Data to Serial Monitor ---
  Serial.print("Temp: "); Serial.print(tempC); Serial.print(" °C | ");
  Serial.print("Current: "); Serial.print(simulatedAmps); Serial.print(" A | ");
  Serial.print("Vibration: "); Serial.print(totalVibration); Serial.println(" m/s^2");

  // --- Check Thresholds & Trigger Alert ---
  if (tempC > TEMP_THRESHOLD || simulatedAmps > CURRENT_THRESHOLD || totalVibration > VIBE_THRESHOLD) {
    faultDetected = true;
    Serial.println(">>> WARNING: MOTOR FAULT DETECTED! <<<");
  }

  // Turn LED on if fault detected, off if normal
  digitalWrite(ALERT_LED_PIN, faultDetected ? HIGH : LOW);

  delay(2000); // Wait 2 seconds before next reading
}