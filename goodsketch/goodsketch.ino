#include <ESP8266WiFi.h>
#include <MQTT.h>

const char ssid[] = "Covenant Battle Network";
const char pass[] = "Westwood1";

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("lightswitch");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  if (topic == "lightswitch" && payload == "turnon") {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  else if (topic == "lightswitch" && payload == "turnoff") {
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  client.begin("192.168.137.247", net);
  client.onMessage(messageReceived);

  connect();
  pinMode(13, OUTPUT);
}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  //  if (millis() - lastMillis > 1000) {
  //    lastMillis = millis();
  //    client.publish("/hello", "world");
  //  }
}

