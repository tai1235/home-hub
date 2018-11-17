var cloud = require('../src/cloud');
var opt = require('getopt');
var fs = require('fs');

var auth_token = '';
var device_id = '';
var user_id = '';
var message = '';
var action = '';
var server_props = '';
var timestamp = false;

try{
    opt.setopt("t:d:u:m:a:b:pr:sv");
} catch (e){
    switch (e.type) {
        case "unknown":
            console.log("Unknown option: -%s", e.opt);
            console.log("Usage: node cloud-example.js [-t <access token>] [-d <device id>] [-u <user id>] " +
                "[-m <JSON type test message>] [-a <JSON type action>] " +
                "[-s for enabling SDR (Secure Device Registered) test] " +
                "[-v for verifying root certificate] [-r <root CA file>]");
            break;
        case "required":
            console.log("Required parameter for option: -%s", e.opt);
            break;
        default:
            console.dir(e);
    }
    process.exit(0);
}

var ssl_config = {
    ca_cert: Buffer.from(""),
    verify_cert: "none",
    se_config: undefined
}

opt.getopt(function (o, p){
    switch(o){
        case 't':
            auth_token = String(p);
            break;
        case 'd':
            device_id = String(p);
            break;
        case 'u':
            user_id = String(p);
            break;
        case 'm':
            message = String(p);
            break;
        case 'a':
            action = String(p);
            break;
        case 'b':
            server_props = String(p);
            break;
        case 'p':
            timestamp = true;
            break;
        case 'r':
            var data = fs.readFileSync(String(p));
            ssl_config.ca_cert = Buffer.from(data);
            break;
        case 's':
            ssl_config.se_config = {
                certificate_identifier: 'artik'
            }
            break;
        case 'v':
            ssl_config.verify_cert = "required";
            break;
        default:
            console.log("Usage: node cloud-example.js [-t <access token>] [-d <device id>] [-u <user id>] " +
                "[-m <JSON type test message>] [-a <JSON type action>] " +
                "[-b <data JSON>] [-p enables timestamp] " +
                "[-s for enabling SDR (Secure Device Registered) test] " +
                "[-v for verifying root certificate] [-r <root CA file>]");
            process.exit(0);
    }
});

var artik_cloud = new cloud(auth_token);

artik_cloud.get_current_user_profile(function(err, response) {
    if (!err) {
        console.log("Get Current User Profile - response: " + JSON.stringify(response));
    } else {
        console.log("Get Current User Profile - error: " + err);
    }
}, ssl_config);

artik_cloud.get_device_token(device_id, function(err, response) {
    if (!err) {
        console.log("Get Device Token - response: " + JSON.stringify(response));
    } else {
        console.log("Get Device Token - error: " + err);
    }
}, ssl_config);

artik_cloud.get_device(device_id, false, function(err, response) {
    if (!err) {
        console.log("Get Device without properties - response: " + JSON.stringify(response));
    } else {
        console.log("Get Device without properties - error: " + err);
    }
}, ssl_config);

artik_cloud.get_device(device_id, true, function(err, response) {
    if (!err) {
        console.log("Get Device with properties - response: " + JSON.stringify(response));
    } else {
        console.log("Get Device with properties - error: " + err);
    }
}, ssl_config);

artik_cloud.get_user_device_types(100, false, 0, user_id, function(err, response) {
    if (!err) {
        console.log("Get User Device Types - response: " + JSON.stringify(response));
    } else {
        console.log("Get User Device Types - error: " + err);
    }
}, ssl_config);

artik_cloud.get_user_devices(100, false, 0, user_id, function(err, response) {
    if (!err) {
        console.log("Get User Devices without properties - response: " + JSON.stringify(response));
    } else {
        console.log("Get User Devices without properties - error: " + err);
    }
}, ssl_config);

artik_cloud.get_user_devices(100, true, 0, user_id, function(err, response) {
    if (!err) {
        console.log("Get User Devices with properties - response: " + JSON.stringify(response));
    } else {
        console.log("Get User Devices with properties - error: " + err);
    }
}, ssl_config);

artik_cloud.send_action(device_id, action, function(err, response) {
    if (!err) {
        console.log("Send action - response: " + JSON.stringify(response));
    } else {
        console.log("Send action - error: " + err);
    }
}, ssl_config);

artik_cloud.send_message(device_id, message, function(err, response) {
    if (!err) {
        console.log("Send message - response: " + JSON.stringify(response));
    } else {
        console.log("Send message - error: " + err);
    }
}, ssl_config);

artik_cloud.update_device_token(device_id, function(err, response) {
    if (!err) {
        console.log("Update Device Token - response: " + JSON.stringify(response));
    } else {
        console.log("Update Device Token - error: " + err);
    }
}, ssl_config);

artik_cloud.delete_device_token(device_id, function(err, response) {
    if (!err) {
        console.log("Delete Device Token - response: " + JSON.stringify(response));
    } else {
        console.log("Delete Device Token - error:" + err);
    }
}, ssl_config);

artik_cloud.set_device_server_properties(device_id, server_props, function(err, response) {
    if (!err) {
        console.log("Set Device Server Properties - response: " + JSON.stringify(response));
    } else {
        console.log("Set Device Server Properties - error: " + err);
    }
}, ssl_config);

artik_cloud.get_device_properties(device_id, timestamp, function(err, response) {
    if (!err) {
        console.log("Get Device Properties - response: " + JSON.stringify(response));
    } else {
        console.log("Get Device Properties - error: " + err);
    }
}, ssl_config);
