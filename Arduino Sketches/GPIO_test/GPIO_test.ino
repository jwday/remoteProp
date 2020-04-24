#include <ESP8266WiFi.h>
#include <Wire.h>
#include <MQTT.h>  // MQTT Library by Joel Gaehwiler, https://github.com/256dpi/arduino-mqtt
#include <Adafruit_MCP23017.h>

// What SSID do you want to connect to, and what's the password?
const char ssid[] = "Apogee";
const char pass[] = "PerformHumanSimulator";

// Variables for commanding a prescribed valve-open time
bool holdOpenSwitch = false;
unsigned long commandedBurnTimeMicros;
unsigned long openedMicros;  // The time at which the valve was commanded to be opened

// Set the publish rate (in milliseconds) for publishing pressure values
float pubRate = 100;
int sensorValue = 0;

// Variables for timekeeping while performing "parallel" processes
unsigned long currentMillis;
unsigned long currentMicros;
unsigned long previousMillis = 0;
unsigned long previousMicros = 0;

// Because I'm using a NodeMCU Amica microcontroller (which uses some other chipset), I
// need to redefine the pins to match how they're labeled on the board.
// For example: Pin D7 on the NodeMCU Amica actually corresponds to Pin 13 in the code.
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

Adafruit_MCP23017 mcp;  // Initialize the MCP23017 class thing
//#define PCF8591 0x48 // Device address = 0x48, 00110000
//byte adc_value0, adc_value1, adc_value2, adc_value3;

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
    client.subscribe("singleValveOn");
    client.subscribe("singleValveOff");
}



void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
    valveID = toInt(payload);
    // Valve ON
    if (topic == "singleValveOn") {
        mcp.digitalWrite(valveID, HIGH);   // turn the LED on (HIGH is the voltage level)
    }

    // Valve OFF
    if (topic == "singleValveOff") {
        mcp.digitalWrite(valveID, LOW);    // turn the LED off by making the voltage LOW
    }
}



void setup() {
    pinMode(2, OUTPUT);

    Serial.begin(9600);
    Wire.begin();
    mcp.begin();      // use default address 0
    mcp.pinMode(0, OUTPUT);
    mcp.pinMode(1, OUTPUT);
    mcp.pinMode(2, OUTPUT);
    mcp.pinMode(3, OUTPUT);
//     Set up MCP23017. 
//          A0  A1  A2  Address
//          L   L   L   0x20
//          H   L   L   0x21
//          L   H   L   0x22
//          H   H   L   0x23
//          L   L   H   0x24
//          etc...
//    Wire.beginTransmission(0x20);  // MCP23017 Address, as specified by the logic applied to Pins A1 A2 A3. 0x20 correspongs to L L L. 
//    Wire.write(0x00); // IODIRA register
//    Wire.write(0x00); // set entire PORT A to output
//    Wire.endTransmission();
//
//    Wire.beginTransmission(0x20);
//    Wire.write(0x01); // IODIRB register
//    Wire.write(0x00); // set entire PORT B to output
//    Wire.endTransmission();
    
    connect();
    client.onMessage(messageReceived);

    // Not implemented yet. Currently using rolling average method.
    // FilterOnePole lowpassFilter ( LOWPASS, filtFreq );
}



void loop() {
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!client.connected()) {
        digitalWrite(12, LOW); // Auto shutoff the valves if client is not connected
        digitalWrite(13, LOW);
        digitalWrite(14, LOW);
        digitalWrite(15, LOW);
        connect(); // Attempt to connect
    }

    currentMillis = millis();

    if (currentMillis - previousMillis >= pubRate) {  
        sensorValue = analogRead(A0);
        if (sensorValue >=512) {
            digitalWrite(2, LOW);
        }
        else {
            digitalWrite(2, HIGH);
        }
        client.publish("float_pressure", String(sensorValue));    
        previousMillis = currentMillis;
    }
}
