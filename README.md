# remoteProp
Sets up MQTT broker on a computer, an MQTT client on a browser, and an MQTT client on an ESP8266 (Current use case is with NodeMCU).
Using the web client you can control twelve valves attached to the ESP8266 by directional control (refer to Thesis-pub for more information), individual valve toggle hold-on, or individual valve press-and-hold.

Requires the following packages prior to use:
- nodejs
- mongodb
- npm
  - `npm audit`
  - `npm install`
  - `npm install mosca`
  - `npm install express`
  - `npm serve-index`

- mqtt


1. Start server from a terminal using the following command:

  `node server`

2. Connect to server using a web browser. If connecting on same device, use the following address:

  `localhost:3000`

  If connecting on different device on same network, you must determine the IP address of the server, such as `192.168.0.103', and type in the following url:

  `192.168.0.103:3000`
