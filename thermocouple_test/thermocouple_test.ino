/*************************************************** 
  This is an example for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Adafruit_MAX31855.h"

// Only software-defined SPI seems to work on the NodeMCU.
// Moreover, using the SPI Bus pins (GPIOs 14, 12, 13, 15) do not work as intended.
// Instead, choose three random GPIO pins and assign them below.
// Just don't connect DO to D6 (GPIO12 / SPI Bus MISO)
// Pin D6 might be broken because a valve control is assigned to it and it too doesn't work

#define MAXDO   4  // DO  -->  D0
#define MAXCS   5  // CS  -->  D1
#define MAXCLK 16  // CLK -->  D2

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);


void setup() {
  Serial.begin(9600);
  Serial.println("MAX31855 on NodeMCU test");
  delay(500);
}

void loop() {
   Serial.println(thermocouple.readCelsius());
   delay(100);
}
