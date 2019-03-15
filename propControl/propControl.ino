#include <ESP8266WiFi.h>
#include <Wire.h>
#include <MQTT.h>

//const char ssid[] = "Apogee";
//const char pass[] = "PerformHumanSimulator";
const char ssid[] = "Covenant Battle Network";
const char pass[] = "Westwood1";

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

unsigned long previousMillis = 0;  // For timekeeping while performing "parallel" processes




void connect() {
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
  client.begin("192.168.43.169", net);
  while (!client.connect("arduino")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected to broker!");
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
}



void setup() {
  Serial.begin(9600);

//  int numberOfNetworks = WiFi.scanNetworks(); 
  
//  for (int i=0; i<numberOfNetworks; i++){
//      Serial.print("Network name: ");
//      Serial.println(WiFi.SSID(i));
//      Serial.println("-----------------------");
//  }

  Wire.begin();
  connect();
  
  client.onMessage(messageReceived);

  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
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

  Wire.beginTransmission(PCF8591);
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

  // String payload = String(adc_value0);
  // println(payload);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 100) {
    client.publish("float_pressure", String(adc_value0));
    client.publish("prop_pressure", String(adc_value3));
    previousMillis = currentMillis;
  }
}
