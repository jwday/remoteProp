# remoteProp
Sets up MQTT broker on a computer, an MQTT client on a browser, and an MQTT client on an ESP8266 (Current use case is with NodeMCU).
Using the web client you can control twelve valves attached to the ESP8266 by directional control (refer to Thesis-pub for more information), individual valve toggle hold-on, or individual valve press-and-hold. The server can be configured to record pressure data coming from the ESP8266 and store the timestamped values in a local .csv file.


## Prerequisties


Requires the following packages prior to use:
- nodejs: `sudo apt-get install nodejs`
- mongodb: `sudo apt-get install mongodb`
- npm: `sudo apt-get install npm`


## Installation

1. Clone the repo

    HTTPS: `git clone https://github.com/jwday/remoteProp.git`
    
    SSH: `git clone git@github.com:jwday/remoteProp.git`

2. Install all dependencies listed in `package.json` by navigating to the repo directory and running:

    `npm install`


## Running the server/message broker

1. Start server from the repo directory by running:

    `node server`
    
    Once running, you should be prompted in-line with the message `Server is up and running`.
    
    All broadcasted messages, including client connections/disconnections, will be shown in the terminal window.

2. Connect to server using a web browser: 

    If connecting on same device, use the following address: `localhost:3000`
    
    If connecting from a different device on the same network, use the IP address of the server: `<server IP address>:3000`
    

## Stopping the server

1. A simple `Ctrl + c` will stop the server.


## Hardware setup

![Hardware setup](https://github.com/jwday/Thesis/blob/master/figures/wireless_net_bw.png)

Requires the following devices at minimum:
- NodeMCU, Feather HUZZAH, or other ESP8266 (Wifi)-enabled microcontroller with Arduino-like code support
- A physical device with which to control with the ESP8266-enabled device, such as an LED or a solenoid valve
- Wifi-enabled computer, such as a laptop or a Raspberry Pi 3+
- Wifi router with ability to assign static IP address to whichever device is running the server/message broker

1. Clone and install this repo to whichever device is going to be the server/message broker

2. Set the DHCP server on the Wifi router to provide a static IP address to the server. For example, `192.168.0.200`.

3. Prepare to upload one of the Arduino sketches from this repo onto your ESP8266-enabled device. `propcontrol_v2.ino` is the most up-to-date sketch and is configured to be used with a NodeMCU v1.1. Before uploading, you must the change the following configuration settings:

    Set the access point name (`const char ssid[] = `)
    
    Set the access point password (`const char pass[] = `)
    
    Set the IP address of the server/message broker (`const char ip_address[] = `)

4. Start the server

5. Power on the ESP8266-enabled device

    If the ESP8266 successfully connects, the server should relay a message such as `client connected propcontroller`
