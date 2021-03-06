#include <ESP8266WiFi.h>        // ESP8266 WiFi interface library
#include <Wire.h>               // I2C interface library
#include <MQTT.h>               // MQTT Library by Joel Gaehwiler, https://github.com/256dpi/arduino-mqtt
#include <Adafruit_MCP23017.h>  // MCP23017 GPIO Expansion board library


// What SSID do you want to connect to, and what's the password?
const char ssid[] = "Apogee";
const char pass[] = "PerformHumanSimulator";


// Variables for commanding a prescribed valve-open time
bool holdOpenSwitch = false;
unsigned long commandedBurnTimeMicros;
unsigned long openedMicros;  // The time at which the valve was commanded to be opened


// Instantiate the valveID variable and set it to 0 (that's GPIO port 0)
int valveID;


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
#define D6 12 // SPI Bus MISO
#define D7 13 // SPI Bus MOSI 
#define D8 15 // SPI Bus SS (CS)
#define D9 3  // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)


// Create an instantce of the MCP23017 GPIO expansion and call it "GPIO_IC".
Adafruit_MCP23017 GPIO_IC;  // Initialize the MCP23017 class thing


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
    client.subscribe("singleValveOn");
    client.subscribe("singleValveOff");
}


void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
    valveID = payload.toInt() - 1;  // Because GPIO incrementation begins at 0 while valveIDs begin at 1
    
    // Valve ON
    if (topic == "singleValveOn") {
        GPIO_IC.digitalWrite(valveID, HIGH);
        digitalWrite(2, LOW);  // Will turn LED ON when an open valve command is sent
    }

    // Valve OFF
    if (topic == "singleValveOff") {
        GPIO_IC.digitalWrite(valveID, LOW);
        digitalWrite(2, HIGH);  // Will turn LED OFF when a close valve command is sent
    }
}


void setup() {
    pinMode(2, OUTPUT);  // This is the LED_BUILTIN pin, used for debugging
    digitalWrite(2, HIGH);  // Default to OFF

    Serial.begin(9600);
    Wire.begin();

//     Set up MCP23017. 
//          A0  A1  A2  Address
//          L   L   L   0
//          H   L   L   1
//          L   H   L   2
//          H   H   L   3
//          etc...

    GPIO_IC.begin(0);      // use address 0 (note that leaving it blank will also set it to address 0)
    for (int i = 0; i <= 11; i++) {
        GPIO_IC.pinMode(i, OUTPUT);
    }
    
    connect();
    client.onMessage(messageReceived);
}



void loop() {
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!client.connected()) {
        for (int i=0; i<15; i++) {
            GPIO_IC.digitalWrite(i, LOW);  // Cycle through all 16 pins and shut them down. NOTE: This is specific to the Adafruit_MCP23017 library.
        }
        connect(); // Attempt to connect
    }
}
