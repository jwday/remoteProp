
var mosca = require('mosca');

var backendSettings = {
  type: 'mongo',
  url: 'mongodb://localhost:27017/mqtt',
  pubsubCollection: 'ascoltatori',
  mongo: {}
};

var moscaSettings = {
  port: 1883,
  backend: backendSettings
};

var express = require("express");
var http = require("http");
var path = require("path");

var app = express()
var srv = http.createServer(app)
var broker = new mosca.Server(moscaSettings);
broker.attachHttpServer(srv);

app.use(express.static(path.dirname(require.resolve("mosca")) + "/public"))
app.use(express.static(path.dirname(require.resolve("jquery"))))
app.use(express.static(path.dirname(require.resolve("bootstrap")) + "/../"))
app.use(express.static(__dirname + "/image"))

// Serve the dashboard at the root directory
app.get("/", function(req, res) {
  res.sendFile(__dirname + '/index.html');
});

app.get("/lightbulb", function(req, res) {
  res.sendFile(__dirname + '/lightbulb.html');
});

srv.listen(3000)

broker.on('clientConnected', function(client) {
    console.log('client connected', client.id);
});

// fired when a message is received
broker.on('published', function(packet, client) {
  // if (['time', 'x', 'y', 'z', 'trigger'].indexOf(packet.topic) < 0) {
     console.log('Published', packet.topic, packet.payload.toString());
  // }
});

broker.on('ready', setup);

// fired when the mqtt broker is ready
function setup() {
  console.log('Server is up and running');
}
