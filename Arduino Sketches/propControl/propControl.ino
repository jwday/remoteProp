#include <ESP8266WiFi.h>
#include <Wire.h>
#include <MQTT.h>  // MQTT Library by Joel Gaehwiler, https://github.com/256dpi/arduino-mqtt

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

#define PCF8591 0x48 // Device address = 0x48, 00110000
byte adc_value0, adc_value1, adc_value2, adc_value3;

WiFiClient net;  // Instantiate an instance of WiFiClient, call it "net"
MQTTClient client;  // Instantiate an instance of MQTTClient, call it "client"



void timedPropel() {
  currentMicros = micros();
  if (currentMicros - openedMicros >= commandedBurnTimeMicros) {
    unsigned long actualBurnTime = currentMicros - openedMicros;
	digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
	digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
	digitalWrite(14, LOW);    // turn the LED off by making the voltage LOW
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
	digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "bckwd") {
	digitalWrite(14, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "panright") {
	digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
	digitalWrite(14, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "panleft") {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "turnleft") {
	digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "turnright") {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
	digitalWrite(14, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  
  else if (topic == "propel" && payload == "turnoff") {
	digitalWrite(12, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
	digitalWrite(14, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(15, LOW);    // turn the LED off by making the voltage LOW
  }

  if (topic == "timedPropel") {
    holdOpenSwitch = true;
    commandedBurnTimeMicros = (unsigned long)(payload.toFloat()*1000000.0);
    Serial.print("Hold-open command received. Thrust for ");
    Serial.print(commandedBurnTimeMicros);
    Serial.println(" microseconds.");
    openedMicros = micros();
	digitalWrite(12, HIGH);   // turn the LED on (HIGH is the voltage level)
	digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
	digitalWrite(14, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
    timedPropel();  // Only sending "openedTime" because the function requires a number to be passed
  }
}



void setup() {
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  
  Serial.begin(9600);
  Wire.begin();

  connect();
  client.onMessage(messageReceived);
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
    // Pressure XDCRs
    Wire.beginTransmission(PCF8591);  // Get data from the A2D Converter (PCF8591)
    Wire.write(0x04); // Send a byte to the PCF8591 to tell it to read all channels IN DIFFERENTIAL MODE
    Wire.endTransmission();
    Wire.requestFrom(PCF8591, 5); // Request three bytes from the PCF8591
  
    while (Wire.available()) {
      adc_value0 = Wire.read(); //This needs two reads to get the value.
      adc_value0 = Wire.read();
      adc_value1 = Wire.read();
      adc_value2 = Wire.read();
      adc_value3 = Wire.read();
    }
    
    // 0 PSIG
    // ADC0 = 26  ADC1 = 22
    // 100 PSIG
    // ADC0 = 221  ADC1 = 217

    // ADC0: 100/(221-26) = 0.51282
    // ADC1: 100/(217-22) = 0.51282
    float float_psig = (adc_value0 - 26)*0.51282;
    float prop_psig = (adc_value1 - 22)*0.51282;

    client.publish("float_pressure", String(float_psig));
    client.publish("prop_pressure", String(prop_psig));
    
    previousMillis = currentMillis;
  }

  if (holdOpenSwitch) {
    timedPropel();
  }
}
