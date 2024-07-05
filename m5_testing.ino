// Dumb Little Program that sends data to MQTT Broker from Sensor Connected to the Grove Plug
#include <M5StickCPlus.h>
#include <Wire.h>
#include "DHT20.h"
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "<NETWORK SSID>";
const char* password = "<NETWORK PASSWORD>";

// MQTT Broker
const char* mqtt_server = "<IP HERE>"; // STRING
const int mqtt_port = <PORT HERE>; // INT 

DHT20 dht;

WiFiClient espClient;
PubSubClient client(espClient);

#define debug Serial

void setup() {
    M5.begin();
    M5.Lcd.setRotation(3);  
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);

    debug.begin(115200);
    debug.println("DHT20 test!");
    Wire.begin();

    if (dht.begin()) {
        debug.println("DHT20 initialized successfully.");
    } else {
        debug.println("Failed to initialize DHT20.");
    }

    // Connect to WiFi
    WiFi.begin(ssid, password);
    debug.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        debug.print(".");
    }
    debug.println("\nWiFi connected");

    // Connect to MQTT broker
    client.setServer(mqtt_server, mqtt_port);
}

void loop() {
    if (!client.connected()) {
        M5.Lcd.fillScreen(RED);
        if (WiFi.status() == WL_CONNECTED) {
            debug.println("Connecting to MQTT broker...");
            if (client.connect("M5StickCPlusClient")) {
                debug.println("Connected to MQTT broker");
                M5.Lcd.fillScreen(BLACK);
            } else {
                delay(5000);
                return;
            }
        }
    }

    dht.read();

    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperature)) {
        debug.println("Failed to read from DHT20 sensor!");
        return;
    }

    debug.print("Humidity: ");
    debug.print(humidity);
    debug.print(" %\t");
    debug.print("Temperature: ");
    debug.print(temperature);
    debug.println(" *C");

    // Read health information
    float batteryVoltage = M5.Axp.GetVbatData() * 1.1 / 1000;
    float batteryCurrent = M5.Axp.GetBatCurrent() * 0.5 / 1000;
    float batteryPower = M5.Axp.GetBatPower();
    float batteryChargeCurrent = M5.Axp.GetBatChargeCurrent();
    float temperatureAXP192 = M5.Axp.GetTempInAXP192();
    float vbusVoltage = M5.Axp.GetVBusVoltage();
    float vbusCurrent = M5.Axp.GetVBusCurrent();

    // Publish sensor data to MQTT broker
    String sensorPayload = String("{\"temperature\": ") + temperature + ", \"humidity\": " + humidity + "}";
    client.publish("m5Stick_1/sensor/data", (char*) sensorPayload.c_str());

    // Publish health data to MQTT broker
    String healthPayload = String("{\"battery_voltage\": ") + batteryVoltage + 
                           ", \"battery_current\": " + batteryCurrent +
                           ", \"battery_power\": " + batteryPower +
                           ", \"battery_charge_current\": " + batteryChargeCurrent +
                           ", \"temperature_axp192\": " + temperatureAXP192 +
                           ", \"vbus_voltage\": " + vbusVoltage +
                           ", \"vbus_current\": " + vbusCurrent + "}";
    client.publish("m5Stick_1/device/health", (char*) healthPayload.c_str());

    delay(1500);
}
