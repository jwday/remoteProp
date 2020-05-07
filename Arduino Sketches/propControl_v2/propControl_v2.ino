// propControl_v2.ino
// Last updated 5/6/2020
//
// To be used with the following components:
// - 1x MCP23017 GPIO expansion IC (Uses I2C interface)
// - 1x MCP3008 4-channel ADC IC (Uses SPI interface)
// - 3x L293D motor controller IC (Controlled by MCP23017)
// - (Optional) 1x HX711 Load Cell amplifier + ADC (Uses I2C interface)
//
// NOTE: NodeMCU pins are redefined in a seperate .ino file that gets automatically
// compiled along with this file.

// Wireless communication libraries
#include <ESP8266WiFi.h>        // WiFi Library
#include <MQTT.h>               // MQTT Library by Joel Gaehwiler, https://github.com/256dpi/arduino-mqtt
#include <Wire.h>               // I2C Library
#include <Adafruit_MCP23017.h>  // MCP23017 GPIO Expansion board library (uses I2C)
#include <SPI.h>                // SPI Library
#include <MCP3XXX.h>            // MCP3XXX ADC library, used for any MCP3XXX series ADC (uses SPI)


// What SSID do you want to connect to, and what's the password?
// Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
// You need to set the IP address of the broker directly.
const char ssid[] = "Apogee";
const char pass[] = "PerformHumanSimulator";
const char ip_address[] = "192.168.43.35";  // Via celluar hotspot to Raspberry Pi broker
//const char ip_address[] = "192.168.0.200";  // Via access point in upstairs lab to Raspberry Pi broker
//const char ip_address[] = "192.168.43.106"; // Via cellular hotspot to LCARS broker


// Variables for controlling pressure XDCR publish rate, ADC read variables, and ADC timing variables
float pubRate = 100;
int adc_value0;
int adc_value1;
unsigned long adc_read_pre;
unsigned long adc_read_post;
unsigned long adc_readTime;


// Variables for commanding a prescribed valve-open time
bool holdOpenSwitch = false;
unsigned long commandedBurnTimeMicros;
unsigned long openedMicros;  // The time at which the valve was commanded to be opened


// Instantiate the valveID variable
int valveID;


// Variables for timekeeping while performing "parallel" processes
unsigned long currentMillis;
unsigned long currentMicros;
unsigned long previousMillis = 0;
unsigned long previousMicros = 0;
int n = 0;
unsigned long averageTime = 0;
unsigned long sampleTime;


// Instantiate all the required objects
WiFiClient net;             // Instantiate a WiFiClient object, call it "net"
MQTTClient client;          // Instantiate an MQTTClient object, call it "client"
MCP3008 adc;                // Instantiate an MCP3008 ADC object, assuming it's connected to the default pins D5, D6, D7, D8, call it "adc"
Adafruit_MCP23017 GPIO_IC;  // Instantiate an MCP23017 GPIO Expansion object


void timedPropel() {
    currentMicros = micros();
    if (currentMicros - openedMicros >= commandedBurnTimeMicros) {
        unsigned long actualBurnTime = currentMicros - openedMicros;
        GPIO_IC.digitalWrite(0, LOW);
        holdOpenSwitch = false;
        client.publish("timedPropelReturn", String(actualBurnTime));
        Serial.print("Opened Time is ");
        Serial.println(openedMicros);
        Serial.print("Closed Time is ");
        Serial.println(currentMicros);
        Serial.println("");
        Serial.print("Thrust command complete. Thrust held for ");
        Serial.print(actualBurnTime);
        Serial.println(" microseconds.");
    }
}


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
    client.begin(ip_address, net);

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

    if (topic == "singleValveOn" || topic == "singleValveOff") {
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
    
    // Prescribed time, currently only available for Valve #1
    if (topic == "timedPropel") {
        holdOpenSwitch = true;
        commandedBurnTimeMicros = (unsigned long)(payload.toFloat() * 1000000.0);
        Serial.print("Hold-open command received. Thrust for ");
        Serial.print(commandedBurnTimeMicros);
        Serial.println(" microseconds.");
        openedMicros = micros();
        GPIO_IC.digitalWrite(0, HIGH);
        timedPropel();  // Only sending "openedTime" because the function requires a number to be passed
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



void avgSampleFreq(float sampleTime) {
    averageTime = (averageTime*(n-1) + sampleTime)/n;
    if (n >= 200) {
        Serial.print("Average loop rate (");
        Serial.print(n);
        Serial.print(" loops): ");
        Serial.print(1000000.0/averageTime);
        Serial.println(" Hz");
        n = 0;
    }
    n++;
}


void loop() {
    // This loop takes ~10.060ms to complete when not reading from ADC (~100 Hz)
    // Add an additional 1.5 to 2ms to include ADC time (~82 Hz)
    // Most of this is caused by the delay() function below
    adc_read_pre = micros();
    client.loop();
    delay(5);  // <- fixes some issues with WiFi stability, but absolutely kills the sample rate

    if (!client.connected()) {
        for (int i=0; i<15; i++) {
            GPIO_IC.digitalWrite(i, LOW);  // Cycle through all 16 pins and shut them down. NOTE: This is specific to the Adafruit_MCP23017 library.
        }
        connect(); // Attempt to connect
    }

    currentMillis = millis();

    if (currentMillis - previousMillis >= pubRate) {
        // This loop takes ~1.5 to 2ms to complete (~500 to 660 Hz)
        // Pressure XDCRs

        
        adc_value0 = adc.analogRead(0);  // Float pressure
        adc_value1 = adc.analogRead(1);  // Prop pressure

        // 0 PSIG
        // ADC0 = 26  ADC1 = 22
        // 100 PSIG
        // ADC0 = 221  ADC1 = 217

        // ADC0: 100/(221-26) = 0.51282
        // ADC1: 100/(217-22) = 0.51282
        float float_psig = (adc_value0 - 26) * 0.51282;
        float prop_psig = (adc_value1 - 22) * 0.51282;

        client.publish("float_pressure", String(float_psig));
        client.publish("prop_pressure", String(prop_psig));

        previousMillis = currentMillis;
    }

    if (holdOpenSwitch) {
        timedPropel();
    }
    
    adc_read_post = micros();
    adc_readTime = adc_read_post - adc_read_pre;
    avgSampleFreq(adc_readTime);
}
