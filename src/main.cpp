#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>

// Network Configuration
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// MQTT Configuration
const char *mqtt_server = "mqtt.thingsboard.cloud";
const char *TOKEN = "TOKEN"; // Replace with your ThingsBoard device token

// Pin Assignments
const int currentPin = 34; // Simulates analog current input

// Global Instances
Adafruit_MPU6050 mpu;
WiFiClient espClient;
PubSubClient client(espClient);

// Initialize WiFi connection
void setup_wifi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi successfully.");
}

// Reconnect to ThingsBoard MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard node...");
    // Attempt to connect using the device token as the username
    if (client.connect("ESP32_MotorClient", TOKEN, NULL)) {
      Serial.println("[DONE]");
    } else {
      Serial.print("[FAILED] [ rc = ");
      Serial.print(client.state());
      Serial.println(" : retrying in 5 seconds]");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // Initialize the MPU6050 sensor
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip.");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 initialized successfully.");
}

void loop() {
  // Ensure the MQTT connection is active
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read the simulated motor current from the potentiometer
  int rawCurrent = analogRead(currentPin);
  float motorCurrent = (rawCurrent / 4095.0) * 50.0;

  // Read vibration and temperature data from the MPU6050
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Serialize the telemetry data into a JSON document
  StaticJsonDocument<200> doc;
  doc["current_amps"] = motorCurrent;
  doc["vibration_x"] = a.acceleration.x;
  doc["vibration_y"] = a.acceleration.y;
  doc["temperature_c"] = temp.temperature;

  char out[200];
  serializeJson(doc, out);

  // Publish the telemetry data to ThingsBoard
  Serial.print("Sending telemetry: ");
  Serial.println(out);
  client.publish("v1/devices/me/telemetry", out);

  // Delay before the next reading
  delay(3000);
}