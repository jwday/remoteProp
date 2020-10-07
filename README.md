# remoteProp
Sets up MQTT broker on a computer, an MQTT client on a browser, and an MQTT client on an ESP8266 (Current use case is with NodeMCU).
Using the web client you can control twelve valves attached to the ESP8266 by directional control (refer to Thesis-pub for more information), individual valve toggle hold-on, or individual valve press-and-hold.

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


## Running the server

1. Start server from the repo directory by running:

    `node server`
    
    Once running, you should be prompted in-line with the message `Server is up and running`.
    
    All broadcasted messages, including client connections/disconnections, will be shown in the terminal window.

2. Connect to server using a web browser: 

    If connecting on same device, use the following address: `localhost:3000`
    
    If connecting from a different device on the same network, use the IP address of the server: `<server IP address>:3000`
    

## Stopping the server

1. A simple `Ctrl + c` will stop the server.
