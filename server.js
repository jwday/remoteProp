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
var record = false;
var data_array = [['time', 'topic', 'payload']];
const testdata = require('fs');


var app = express();
var srv = http.createServer(app);
var broker = new mosca.Server(moscaSettings);
broker.attachHttpServer(srv);

app.use(express.static(path.dirname(require.resolve("mosca")) + "/public"));
app.use(express.static(path.dirname(require.resolve("jquery"))));
app.use(express.static(path.dirname(require.resolve("bootstrap")) + "/../"));
app.use(express.static(__dirname));
app.use(express.static(__dirname + "/image"));
app.use(express.static(__dirname + "/data"));

// Serve the dashboard at the root directory
app.get("/", function(req, res) {
    res.sendFile(__dirname + '/index.html');
});

app.get("/data", function(req, res) {
    res.sendFile(__dirname + '/data/testdata');
});

srv.listen(3000);

broker.on('clientConnected', function(client) {
    console.log('client connected', client.id);
});

broker.on('clientDisconnected', function(client) {
    broker.publish({topic: 'propel', payload: 'turnoff', qos: 2})
});

// fired when a message is received
broker.on('published', function(packet, client) {
    console.log('Published', packet.topic, packet.payload.toString());

    // if topic = timedPropel && record = false
        // record = true
    // if record = true
        // save the timestamp, packet.topic, and packet.payloadn to an array
    // if topic = timedPropelReturn && record = true
        // record = false

    if (packet.topic=="timedPropel" && Boolean(record)==false) {
        record = true;
        console.log('Recording data');
        // var timestamp_array = [];
        // var topic_array = [];
        // var payload_array = [];
    };

    if (Boolean(record)==true) {
        // timestamp_array.push(Date.now());
        // topic_array.push(packet.topic);
        // payload_array.push(packet.payload.toString());
        data_array.push([Date.now(), packet.topic, packet.payload.toString()])
    };

    if (packet.topic=="timedPropelReturn" && Boolean(record)==true) {
        record = false;
        console.log('Stopped recording data');

        testdata.writeFile(__dirname + '/data/testdata.txt', arrayToCSV(data_array), function(err) {
            if(err) {
                return console.log(err);
            }
            console.log("The file was saved!");
        });     
    };    
});

broker.on('ready', setup);

// fired when the mqtt broker is ready
function setup() {
    console.log('Server is up and running');
};

function arrayToCSV(twoDiArray) {
    //  Modified from: http://stackoverflow.com/questions/17836273/
    //  export-javascript-data-to-csv-file-without-server-interaction
    var csvRows = [];
    for (var i = 0; i < twoDiArray.length; ++i) {
        // for (var j = 0; j < twoDiArray[i].length; ++j) {
        //     twoDiArray[i][j] = '\"' + twoDiArray[i][j] + '\"';  // Handle elements that contain commas
        // }
        csvRows.push(twoDiArray[i].join(','));
    }

    var csvString = csvRows.join('\r\n');
    return csvString;

    // var a         = document.createElement('a');
    // a.href        = 'data:attachment/csv,' + csvString;
    // a.target      = '_blank';
    // a.download    = 'myFile.csv';

    // document.body.appendChild(a);
    // a.click();
    // Optional: Remove <a> from <body> after done
};


