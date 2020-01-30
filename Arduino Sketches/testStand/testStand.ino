#include <HX711.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <MQTT.h>  // MQTT Library by Joel Gaehwiler, https://github.com/256dpi/arduino-mqtt
#include <SPI.h>
#include "Adafruit_MAX31855.h"
#include "HX711.h" //This library can be obtained here http://librarymanager/All#Avia_HX711


const char ssid[] = "Apogee";
const char pass[] = "PerformHumanSimulator";

// Variables for commanding a prescribed valve-open time
bool holdOpenSwitch = false;
unsigned long commandedBurnTimeMicros;
unsigned long openedMicros;  // The time at which the valve was commanded to be opened

// Variabkles for timekeeping while performing "parallel" processes
unsigned long currentMillis;
unsigned long currentMicros;
unsigned long previousMillis = 0;
unsigned long previousMicros = 0;

// Set the publish rate (in milliseconds) for publishing pressure values
float pubRate = 100;


// Because I'm using a NodeMCU Amica microcontroller (which uses some other chipset), I
// need to redefine the pins to match how they're labeled on the board.
// For example: Pin D7 on the NodeMCU Amica actually corresponds to Pin 13 in the code.
// I don't know why this is a thing, but it is.
// The reassignment below was pulled from a github discussion forum.
// Now, instead of referring to Pin 13, call it Pin D7 (for example).
//#define D0 16 
//#define D1 5  // I2C Bus SCL (clock), also the clock for the thermocouple
//#define D2 4  // I2C Bus SDA (data) Chip select (CS) pin for thermocouple
//#define D3 0  // Data out pin (DO) for thermocouple
//#define D4 2  // Same as "LED_BUILTIN", but inverted logic
//#define D5 14 // SPI Bus SCK (clock)
//#define D6 12 // SPI Bus MISO NOT WORKING
//#define D7 13 // SPI Bus MOSI 
//#define D8 15 // SPI Bus SS (CS)
//#define D9 3  // RX0 (Serial console)
//#define D10 1 // TX0 (Serial console)
#define MAXDO   0
#define MAXCS   16
#define MAXCLK  2

// #define calibration_factor -6522 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define calibration_factor -7036

#define LOADCELL_DOUT_PIN  14
#define LOADCELL_SCK_PIN  12

// Init the scale
HX711 scale;

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

#define PCF8591 0x48 // Device address = 0x48, 00110000
byte adc_value0, adc_value1, adc_value2, adc_value3;

WiFiClient net;  // Instantiate an instance of WiFiClient, call it "net"
MQTTClient client;  // Instantiate an instance of MQTTClient, call it "client"


void timedPropel() {
  currentMicros = micros();
  if (currentMicros - openedMicros >= commandedBurnTimeMicros) {
    unsigned long actualBurnTime = currentMicros - openedMicros;
//    digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
//    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
//    digitalWrite(14, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(15, LOW);    // turn the LED off by making the voltage LOW
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

  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  if(WiFi.status() == WL_CONNECTED) {
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
  client.begin("192.168.0.200", net);  // User for connecting via Apogee hotspot in the upstairs lab to Raspberry Pi
//  client.begin("192.168.43.68", net);  // Use for connecting via 4G tether to Raspberry Pi
  // client.begin("192.168.43.106", net);  // Use for connecting via 4G tether to LCARS 
  
  while (!client.connect("propcontroller")) {
  Serial.print(".");
  delay(1000);
  }

  Serial.println("Connected to broker!");
  client.subscribe("propel");
  client.subscribe("timedPropel");
}



void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  if (topic == "propel" && payload == "fwd") {
//    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "bckwd") {
//    digitalWrite(14, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "panright") {
//    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
//    digitalWrite(14, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "panleft") {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "turnleft") {
//    digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "turnright") {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
//    digitalWrite(14, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  
  else if (topic == "propel" && payload == "turnoff") {
//    digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
//    digitalWrite(14, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(15, LOW);    // turn the LED off by making the voltage LOW
  }

  if (topic == "timedPropel") {
    holdOpenSwitch = true;
    commandedBurnTimeMicros = (unsigned long)(payload.toFloat()*1000000.0);
    Serial.print("Hold-open command received. Thrust for ");
    Serial.print(commandedBurnTimeMicros);
    Serial.println(" microseconds.");
    openedMicros = micros();
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
//    digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
    timedPropel();  // Only sending "openedTime" because the function requires a number to be passed
  }
}



void setup() {
//  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
//  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  
  Serial.begin(9600);
  Wire.begin();
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0

  
  connect();
  client.onMessage(messageReceived);
}



void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
//    digitalWrite(12, LOW); // Auto shutoff the valves if client is not connected
    digitalWrite(13, LOW);
//    digitalWrite(14, LOW);
    digitalWrite(15, LOW);
    connect(); // Attempt to connect
  }
  
  currentMillis = millis();
  
  if (currentMillis - previousMillis >= pubRate) {
    // Pressure XDCRs
    Wire.beginTransmission(PCF8591);  // Get data from the A2D Converter (PCF8591)
    Wire.write(0x04); // Send a byte to the PCF8591 to tell it to read all channels IN DIFFERENTIAL MODE
//    Wire.write(0x00); // Read channel 0
//    Wire.write(0x01); // Read channel 1
    Wire.endTransmission();
    Wire.requestFrom(PCF8591, 5); // Request three bytes from the PCF8591
  
    while (Wire.available()) {
      adc_value0 = Wire.read(); //This needs two reads to get the value.
      adc_value0 = Wire.read();
      adc_value1 = Wire.read();
      adc_value2 = Wire.read();
      adc_value3 = Wire.read();
    }
    
//    Serial.print("ADC 0: ");
//    Serial.print(adc_value0);
//    Serial.print("          ADC 1: ");
//    Serial.print(adc_value1);
//    Serial.print("          ADC 2: ");
//    Serial.print(adc_value2);
//    Serial.print("          ADC 3: ");
//    Serial.println(adc_value3);
    
    // 0 PSIG
    // ADC0 = 26  ADC1 = 22
    // 100 PSIG
    // ADC0 = 221  ADC1 = 217

    // ADC0: 100/(221-26) = 0.51282
    // ADC1: 100/(217-22) = 0.51282
    float float_psig = (adc_value0 - 26)*0.51282;
    float prop_psig = (adc_value1 - 22)*0.51282;
    
//    Serial.print("Float PSI (0): ");
//    Serial.print(float_psig);
//    Serial.print("          Prop PSI (1): ");
//    Serial.println(prop_psig);


    // Thermocouple
    //float temp = thermocouple.readCelsius(); // Might have to change double to float if it doesn't work

    // Load Cell
    float weight = scale.get_units(); //scale.get_units() returns a float

    client.publish("float_pressure", String(float_psig));
    client.publish("prop_pressure", String(prop_psig));
//    client.publish("exit_temp", String(temp)); // Send through new topic
    client.publish("loadcell_weight", String(weight));
    
    previousMillis = currentMillis;
  }


  if (holdOpenSwitch) {
    timedPropel();
  }
}
