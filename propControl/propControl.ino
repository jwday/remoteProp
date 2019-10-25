#include <ESP8266WiFi.h>
#include <Wire.h>
#include <MQTT.h>  // MQTT Library by Joel Gaehwiler, https://github.com/256dpi/arduino-mqtt

const char ssid[] = "Apogee";
const char pass[] = "PerformHumanSimulator";

// Variables for commanding a prescribed valve-open time
bool holdOpenSwitch = false;
float commandedBurnTime;
unsigned long openedMillis;  // The time at which the valve was commanded to be opened

// Variabkles for timekeeping while performing "parallel" processes
unsigned long currentMillis;
unsigned long previousMillis = 0; 

// Set the publish rate (in milliseconds) for publishing pressure values
float pubRate = 200;


// Because I'm using a NodeMCU Amica microcontroller (which uses some other chipset), I need to redefine the pins to match how they're labeled on the board.
// For example: Pin D7 on the NodeMCU Amica actually corresponds to Pin 13 in the code.
// I don't know why this is a thing, but it is.
// The reassignment below was pulled from a github discussion forum.
// Now, instead of referring to Pin 13, call it Pin D7 (for example).
#define D0 16
#define D1 5  // I2C Bus SCL (clock)
#define D2 4  // I2C Bus SDA (data)
#define D3 0
#define D4 2  // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO 
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3  // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)

#define PCF8591 0x48 // Device address = 0x48
byte adc_value0, adc_value1, adc_value2, adc_value3;

WiFiClient net;  // Instantiate an instance of WiFiClient, call it "net"
MQTTClient client;  // Instantiate an instance of MQTTClient, call it "client"





void timedPropel() {
  if (currentMillis - openedMillis >= commandedBurnTime) {
    unsigned long actualBurnTime = currentMillis - openedMillis;
    digitalWrite(D5, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(D6, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(D7, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
    holdOpenSwitch = false;
    client.publish("timedPropelReturn", String(actualBurnTime));
    Serial.print("Thrust command complete. Thrust held for ");
    Serial.print(actualBurnTime);
    Serial.println(" seconds.");
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
  // client.begin("192.168.0.200", net);
  client.begin("192.168.43.68", net);
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
    digitalWrite(D5, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "bckwd") {
    digitalWrite(D6, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(D7, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "panright") {
    digitalWrite(D5, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(D6, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "panleft") {
    digitalWrite(D7, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "turnleft") {
    digitalWrite(D5, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(D7, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  else if (topic == "propel" && payload == "turnright") {
    digitalWrite(D6, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  
  else if (topic == "propel" && payload == "turnoff") {
    digitalWrite(D5, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(D6, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(D7, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
  }

  else if (topic == "timedPropel") {
    holdOpenSwitch = true;
    commandedBurnTime = payload.toFloat();
    Serial.print("Hold-open command received. Thrust for ");
    Serial.print(commandedBurnTime);
    Serial.println(" seconds.");
    openedMillis = millis();
    digitalWrite(D5, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
    timedPropel();  // Only sending "openedTime" because the function requires a number to be passed
  }
}



void setup() {
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  
  Serial.begin(9600);
  Wire.begin();
  
  connect();
  client.onMessage(messageReceived);
}



void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    digitalWrite(D5, LOW); // Auto shutoff the valves if client is not connected
    digitalWrite(D6, LOW);
    digitalWrite(D7, LOW);
    digitalWrite(D8, LOW);
    connect(); // Attempt to connect
  }
  
  currentMillis = millis();
  
  if (currentMillis - previousMillis >= pubRate) {
    Wire.beginTransmission(PCF8591);  // Get data from the A2D Converter (PCF8591)
    Wire.write(0x04); // Send a byte to the PCF8591 to tell it to read all channels
    Wire.endTransmission();
    Wire.requestFrom(PCF8591, 5); // Request one byte from the PCF8591, which should correspond to the reading of Channel 3
  
    while (Wire.available()) {
      adc_value0 = Wire.read(); //This needs two reads to get the value.
      adc_value0 = Wire.read();
      adc_value1 = Wire.read();
      adc_value2 = Wire.read();
      adc_value3 = Wire.read();
    }

    float float_psi = round(adc_value0*0.478 + 3.91 - 14.8);
    float prop_psi = round(adc_value1*0.478 + 3.91 - 14.8);
    
    client.publish("float_pressure", String(float_psi));
    client.publish("prop_pressure", String(prop_psi));
    previousMillis = currentMillis;
  }

  if (holdOpenSwitch) {
    timedPropel();
  }
}
