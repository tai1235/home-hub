/**
 * Author: TaiNV
 * Date Created: 2019/03/09
 * Module: http-client
 * Description: Communication with Server using HTTP
 */

// Dependencies
let http = require('http');
let https = require('https');
let mqtt = require('mqtt');
let config = require('../config');
let EventEmitter = require('events');
let Logger = require('../libraries/system-log');

let logger = new Logger(__filename);

class ServerCommunicator extends EventEmitter {
    constructor(params) {
        super();
        this.serverOpts = params.server;
        this.mqttOpts = {
            host: params.cloudMQTT.host,
            port: params.cloudMQTT.port,
            username: params.cloudMQTT.authentication.user,
            password: params.cloudMQTT.authentication.password,
            clientId: '',
            protocol: 'mqtt'
        }
    }

    identifyHub(callback) {
        const payload = {

        };

        const payloadString = JSON.stringify(payload);

        const requestDetail = {
            'protocol': 'http:',
            'hostname': this.serverOpts.host,
            'method': 'POST',
            'path': '/v1/charges',
            'auth': this.serverOpts.authentication.key,
            'headers': {
                'Content-Type': 'application/json',
                'Content-Length': Buffer.byteLength(payloadString)
            }
        };

        let request = https.request(requestDetail, response => {
            if (response.statusCode === 200 || response.statusCode === 201) {
                response.on('data', data => {
                    callback(JSON.parse(data));
                })
            } else {
                logger.error('Status code: ' + response.statusCode);
                callback({ error: response.statusCode });
            }
        });

        request.on('error', e => {
            callback({ error: e });
        });
        request.write(payloadString);
        request.end();
    }

    connectCloudMQTT(clientID) {
        this.mqttOpts.clientId = clientID;
        this.client = mqtt.connect(this.mqttOpts);
    }

    onConnect(callback) {
        this.client.on('connect', () => {

        })
    }
}

