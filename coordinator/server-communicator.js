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
            protocol: 'http:',
            hostname: this.serverOpts.host,
            method: 'POST',
            path: '/v1/charges',
            auth: this.serverOpts.authentication.key,
            headers: {
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
            this.client.subscribe([
                this.mqttOpts.clientId + '/devices/+',
                this.mqttOpts.clientId + '/rules/+',
                this.mqttOpts.clientId + '/groups/+'
            ]);
            callback();
        })
    }

    process() {
        this.client.on('message', (topic, message) => {
            let messageData = JSON.parse(message.toString());
            let topicLevel = topic.split('/');
            switch (topicLevel[1]) {
                case 'devices': {
                    this._handleDeviceCommand(topicLevel[2], messageData);
                } break;
                case 'rules': {
                    this._handleRuleCommand(topicLevel[2], messageData);
                } break;
                case 'groups': {
                    this._handleGroupCommand(topicLevel[2], messageData);
                } break;
            }
        })
    }

    _handleDeviceCommand(topic, command) {
        switch (topic) {
            case 'requests': {
                switch (command.type) {
                    case 'info': {
                        this.emit('server-device-info', command.data);
                    } break;
                    case 'add': {
                        this.emit('server-device-search', command.data);
                    } break;
                    case 'update': {
                        this.emit('server-device-remove', command.data);
                    } break;
                    case 'remove': {
                        this.emit('server-device-update', command.data);
                    } break;
                    case 'control': {
                        this.emit('server-device-control', command.data);
                    } break;
                }
            } break;
            case 'responses': {

            } break;
        }
    }

    _handleRuleCommand(topic, command) {
        switch (topic) {
            case 'requests': {
                switch (command.type) {
                    case 'add': {
                        this.emit('server-rule-search', command.data);
                    } break;
                    case 'info': {
                        this.emit('server-rule-info', command.data);
                    } break;
                    case 'config': {
                        this.emit('server-rule-update', command.data);
                    } break;
                    case 'remove': {
                        this.emit('server-rule-remove', command.data);
                    } break;
                    case 'control': {
                        this.emit('server-rule-control', command.data);
                    } break;
                    default: {
                        logger.warn('Invalid request from server');
                    }
                }
            } break;
            case 'responses': {

            } break;
        }
    }
    _handleGroupCommand(topic, command) {
        switch (topic) {
            case 'requests': {
                switch (command.type) {
                    case 'add': {
                        this.emit('server-group-search', command.data);
                    } break;
                    case 'info': {
                        this.emit('server-group-info', command.data);
                    } break;
                    case 'config': {
                        this.emit('server-group-update', command.data);
                    } break;
                    case 'remove': {
                        this.emit('server-group-remove', command.data);
                    } break;
                    case 'control': {
                        this.emit('server-group-control', command.data);
                    } break;
                }
            } break;
            case 'responses': {

            } break;
        }
    }

    response(module, message) {
        let topic = this.mqttOpts.clientId + '/' + module + '/responses';
        this.client.publish(topic, message);
    }
}

module.exports = ServerCommunicator;

