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
var serveIndex = require('serve-index')
var http = require("http");
var path = require("path");
var fs = require('fs');

var record = false;
var prop_data_array = [[Date()]];
var float_data_array = [[Date()]];



var app = express();
var srv = http.createServer(app);
var broker = new mosca.Server(moscaSettings);
broker.attachHttpServer(srv);

app.use(express.static(path.dirname(require.resolve("mosca")) + "/public"));
app.use(express.static(path.dirname(require.resolve("jquery"))));
app.use(express.static(path.dirname(require.resolve("bootstrap")) + "/../"));
app.use(express.static(__dirname));
app.use(express.static(__dirname + '/image'));
app.use('/image', express.static(__dirname + '/image'), serveIndex(__dirname + '/image', {'icons': true}));
app.use('/data', express.static(__dirname + '/data'), serveIndex(__dirname + '/data', {'icons': true}));

// Serve the dashboard at the root directory
app.get("/", function(req, res) {
    res.sendFile(__dirname + '/index.html');
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
        
        // Reset the data arrays in case they've already been filled from a previous recording
        prop_data_array = [[Date()]];
        float_data_array = [[Date()]];

        // Append column names
        prop_data_array.push(['Time (s)', 'Topic', 'Payload'])
        float_data_array.push(['Time (s)', 'Topic', 'Payload'])
        
        stopwatch_ref = Date.now();
    };

    if (Boolean(record)==true) {
        stopwatch = Date.now()-stopwatch_ref;
        if (packet.topic=="prop_pressure") {
            prop_data_array.push([stopwatch/1000, packet.topic, parseFloat(packet.payload.toString()).toFixed(1)])
        };
        if (packet.topic=="float_pressure") {
            float_data_array.push([stopwatch/1000, packet.topic, parseFloat(packet.payload.toString()).toFixed(1)])
        };
    };

    if (packet.topic=="timedPropelReturn" && Boolean(record)==true) {
        record = false;
        console.log('Stopped recording data');
        var filename = formatDateTime(Date())

        fs.writeFile(__dirname + '/data/' + filename + '_prop_data.csv', arrayToCSV(prop_data_array), function(err) {
            if(err) {
                return console.log(err);
            }
            console.log("Prop data was saved!");
        });

        fs.writeFile(__dirname + '/data/' + filename + '_float_data.csv', arrayToCSV(float_data_array), function(err) {
            if(err) {
                return console.log(err);
            }
            console.log("Float data was saved!");
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


function formatDateTime(date) {
    var d = new Date(date),
        month = '' + (d.getMonth() + 1),
        day = '' + d.getDate(),
        year = d.getFullYear();
        hour = '' + d.getHours();
        minute = '' + d.getMinutes();
        second = '' + d.getSeconds(); 

    if (month.length < 2) 
        month = '0' + month;
    if (day.length < 2) 
        day = '0' + day;

    return [year+month+day, hour+minute+second].join('_');
}