#include <ESP8266WiFi.h>        // ESP8266 WiFi interface library
#include <Wire.h>               // I2C interface library
#include <MQTT.h>               // MQTT Library by Joel Gaehwiler, https://github.com/256dpi/arduino-mqtt
#include <MCP3XXX.h>            // MCP3XXX ADC library, used for any MCP3XXX series ADC


// What SSID do you want to connect to, and what's the password?
const char ssid[] = "Apogee";
const char pass[] = "PerformHumanSimulator";


// Set the publish rate (in milliseconds) for publishing pressure values, and initialize sensorValue
float pubRate = 100;
int sensorValue;


// Variables for timekeeping while performing "parallel" processes
unsigned long currentMillis;
unsigned long previousMillis = 0;


// Because I'm using a NodeMCU microcontroller (which uses some other chipset), I
// need to redefine the pins to match how they're labeled on the board.
// For example: Pin D7 on the NodeMCU actually corresponds to Pin 13 in the code.
// I don't know why this is a thing, but it is.
// The reassignment below was pulled from a github discussion forum.
// Now, instead of referring to Pin 13, call it Pin D7 (for example).
#define D0 16
#define D1 5  // I2C Bus SCL (clock), also the clock for the thermocouple
#define D2 4  // I2C Bus SDA (data) Chip select (CS) pin for thermocouple
#define D3 0  // Data out pin (DO) for thermocouple
#define D4 2  // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO NOT WORKING
#define D7 13 // SPI Bus MOSI 
#define D8 15 // SPI Bus SS (CS)
#define D9 3  // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)


// Create an instance of the MCP3008 ADC, assuming it's connected to the default pins D5, D6, D7, D8
MCP3008 adc;


// Create a WiFiClient and MQTTClient instance
WiFiClient net;  // Instantiate an instance of WiFiClient, call it "net"
MQTTClient client;  // Instantiate an instance of MQTTClient, call it "client"


void connect() {
    WiFi.hostname("propcontroller");
    WiFi.begin(ssid, pass);
    Serial.print("Attempting to connect to ");
    Serial.println(ssid);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nWiFi connected to ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.println("");
    }

    Serial.println("Connecting to broker...");
    // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
    // You need to set the IP address of the broker directly.

    // New IP Address, old one was 192.168.0.200
    //  client.begin("192.168.0.200", net);  // User for connecting via Apogee hotspot in the upstairs lab to Raspberry Pi
    client.begin("192.168.43.35", net);  // Use for connecting via 4G tether to Raspberry Pi
    //  client.begin("192.168.43.106", net);  // Use for connecting via 4G tether to LCARS

    while (!client.connect("propcontroller")) {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("Connected to broker!");
}



void setup() {
    Serial.begin(9600);
    adc.begin();    // Use the default SPI hardware interface
    connect();
}


void loop() {
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    currentMillis = millis();

    if (currentMillis - previousMillis >= pubRate) {  
        sensorValue = adc.analogRead(0);
        client.publish("float_pressure", String(sensorValue));
        previousMillis = currentMillis;
    }
}
