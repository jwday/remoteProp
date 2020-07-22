// SERVER SETTINGS
var mosca = require('mosca'); 	// This is the MQTT Broker (the thing that receives messages and publishes them to specific topics)
								// It is being set up as a Node.js app (as opposed to running it standalone)
								// NOTE: MOSCA IS UNMAINTAINED. Per https://github.com/moscajs/mosca, they suggest moving to Aedes

var moscaSettings = {			// This is where/how you set up the Mosca (MQTT Broker) settings
    port: 1883,					// Default port is 1883. Make sure it's open on your router.
    backend: backendSettings
};

var backendSettings = { 		// This is the database. Not really needed at this moment but can be useful for storing
    type: 'mongo',
    url: 'mongodb://localhost:27017/mqtt',
    pubsubCollection: 'ascoltatori',
    mongo: {}
};

var express = require("express");  // This is the Node.js framework that provides all the functions to make this server work

var serveIndex = require('serve-index')
var http = require("http");
var path = require("path");
var fs = require('fs');


// SWITCH VARIABLES AND DATA ARRAYS
var record = false;
var record_tail = false;
var valve_open = false;

var valve_open_time = 0;
var valve_close_time = 0;

var prop_data_array = [[Date()]];
var float_data_array = [[Date()]];
var temp_data_array  = [[Date()]];
var loadcell_data_array  = [[Date()]];
// var all_data_array  = [[Date()]];


// START SERVER
var app = express();
var srv = http.createServer(app);
var broker = new mosca.Server(moscaSettings);	// Init the Mosca MQTT broker
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



// MANAGE CLIENTS
broker.on('clientConnected', function(client) {
    console.log('client connected', client.id);
});

broker.on('clientDisconnected', function(client) {  // Failsafe to close all valves in case client disconnects
	// broker.publish({topic: 'propel', payload: 'turnoff', qos: 2})
});



// MESSAGE RECEIVED
broker.on('published', function(packet, client) {
    console.log('Published', packet.topic, packet.payload.toString());

    // Run this if recording, regardless of any other state
    if (Boolean(record)==true) {
        stopwatch = Date.now()-stopwatch_ref;  // Milliseconds
        recordData(stopwatch, packet.topic, packet.payload)
    };

    // Burn phases:
    // 1) 0.000s: Initilization command received from interface via broker, initiate recording
    // 2) 0.500s: Send "fire" command
    // 3) X.XXXs: "Fire complete" message received
    // 4) X.XXX+2s: Stop recording


    // If a "Fire" command is received and we're not recording or firing
    if (packet.topic=="timedPropelInit" && Boolean(record)==false &&  Boolean(valve_open)==false) {     
        // Reset the data arrays in case they've already been filled from a previous recording
        prop_data_array = [[Date()]];
        float_data_array = [[Date()]];
        temp_data_array  = [[Date()]];
        loadcell_data_array  = [[Date()]];
        // all_data_array = [[Date()]];

			
        // Append column names to the data arrays
        prop_data_array.push(['Time (s)', 'Prop Pressure (psig)']);
        float_data_array.push(['Time (s)', 'Float Pressure (psig)']);
        temp_data_array.push(['Time (s)', 'Exit Temperature (Celsius)']);
        loadcell_data_array.push(['Time (s)', 'Weight (?)']);
        // all_data_array.push(['Time (s)', 'Prop Pressure (psig)', 'Float Pressure (psig)', 'Exit Temperature (Celsius)', 'Weight (?)'])
          
        stopwatch_ref = Date.now();
        record = true;

        console.log('Recording data');
        console.log('Sending valve-open command in 0.5 seconds');

        burn_time = packet.payload.toString();

        setTimeout(function() {
            valve_open = true;
            valve_open_time = Date.now()-stopwatch_ref;  // Milliseconds

            // Send the "fire" command
            console.log('Sending valve-open command now');
            broker.publish({topic: 'timedPropel', payload: burn_time, qos: 2});  // 'timedPropel' expects burn time to be in seconds
        }, 500);
    };


    // If a "Completed firing" message is received and we're recording
    if (packet.topic=="timedPropelReturn" && Boolean(record)==true  && Boolean(valve_open)==true && Boolean(record_tail)==false) {
        // valve_close_time = Date.now()-stopwatch_ref;  // Milliseconds
        valve_open = false;
        console.log('Valve closed, recording data for 5 more seconds');
        
        setTimeout(function() {
            record = false;
            console.log('Stopped recording data');
            var runtime = formatDateTime(Date())
            var data_name = {prop_data: prop_data_array, float_data: float_data_array, temp_data: temp_data_array, loadcell_data: loadcell_data_array};

    
            for (const data in data_name) {
                fs.writeFile(__dirname + '/data/' + runtime + '_' + data + '.csv', arrayToCSV(data_name[data]), function(err) {
                    if(err) {
                        return console.log(err);
                    }
                    else {
                        console.log(data + ' was saved!');
                    }
                });
            }
        }, 5000)
    }



    if (Boolean(record)==true && Boolean(valve_open)==false && (Date.now()-stopwatch_ref) >= ((burn_time*1000)+2000)) {


        // fs.writeFile(__dirname + '/data/' + filename + '_prop_data.csv', arrayToCSV(prop_data_array), function(err) {
        //     if(err) {
        //         return console.log(err);
        //     }
        //     else {
        //         console.log("Float data was saved!");
        //     }
        // });

        // fs.writeFile(__dirname + '/data/' + filename + '_float_data.csv', arrayToCSV(float_data_array), function(err) {
        //     if(err) {
        //         return console.log(err);
        //     }
        //     else {
        //         console.log("Float data was saved!");
        //     }
        // });
    };    
});

broker.on('ready', setup);

// fired when the mqtt broker is ready
function setup() {
    console.log('Server is up and running');
};

function arrayToCSV(twoDiArray) {
    //  Modified from: http://stackoverflow.com/questions/17836273/
    var csvRows = [];
    for (var i = 0; i < twoDiArray.length; ++i) {
        csvRows.push(twoDiArray[i].join(','));
    }

    var csvString = csvRows.join('\r\n');
    return csvString;
};


function formatDateTime(date) {
    var d = new Date(date),
        month = '' + (d.getMonth() + 1),
        day = '' + d.getDate(),
        year = d.getFullYear();
        hour = '' + d.getHours();
        minute = '' + d.getMinutes();
        second = '' + d.getSeconds(); 

    if (month.length < 2) {
        month = '0' + month;
    };
        
    if (day.length < 2) {
        day = '0' + day;
    };

    if (hour.length < 2) {
        hour = '0' + hour;
    }

    if (minute.length < 2) {
        minute = '0' + minute;
    }

    if (second.length < 2) {
        second = '0' + second;
    }

    return [year+month+day, hour+minute+second].join('_');
}


function recordData (time, topic, payload) {
    if (topic=="prop_pressure") {
        prop_data_array.push([time/1000, parseFloat(payload.toString()).toFixed(1)])
    };

    if (topic=="float_pressure") {
        float_data_array.push([time/1000, parseFloat(payload.toString()).toFixed(1)])
    };

    if (topic=="exit_temp") {
        temp_data_array.push([time/1000, parseFloat(payload.toString()).toFixed(1)])
    }

    if (topic=="loadcell_weight") {
        loadcell_data_array.push([time/1000, parseFloat(payload.toString()).toFixed(1)])
    }
}



