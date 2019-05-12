/**
 * Author: TaiNV
 * Date Created: 2019/04/15
 * Module: local-server
 * Description: A MQTT broker to handle direct connection to this hub
 */

// Dependencies
let mosca = require('mosca');
let mqtt = require('mqtt');
let EventEmitter = require('events');
let config = require('../../libraries/config');
let Logger = require('../../libraries/system-log');

let logger = new Logger(__filename);

class LocalServer extends EventEmitter {
    constructor() {
        super();
        this.port = config.mosca.port;
        this.user = config.mosca.username;
        this.password = config.mosca.password;
        this.hubTopics = {
            request: 'user/' + this.user + '/hub/request/',
            response: 'user/' + this.user + '/hub/response/',
            status: 'user/' + this.user + '/hub/status/'
        };
        this.appTopics = {
            request: 'user/' + this.user + '/app/request/',
            response: 'user/' + this.user + '/app/response/',
            control: 'user/' + this.user + '/app/control/'
        };
        this.authenticate = (client, username, password, callback) => {
            let authorized = (username === this.user && password.toString() === this.password);
            if (authorized) client.user = username;
            callback(null, authorized);
        };
        this.authorizePublish = (client, topic, payload, callback) => {
            let topicLevel = topic.split('/');
            callback(null, topicLevel[0] === 'user' && topicLevel[1] === client.user);
        };
        this.authorizeSubscribe = (client, topic, callback) => {
            let topicLevel = topic.split('/');
            callback(null, topicLevel[0] === 'user' && topicLevel[1] === client.user);
        };
    }

    start() {
        logger.debug('Creating mosca broker');
        this.server = new mosca.Server({
            port: this.port
        });
        this.server.on('ready', () => {
            logger.debug('Mosca broker ready to connect');
            this.server.authenticate = this.authenticate;
            this.server.authorizePublish = this.authorizePublish;
            this.server.authorizeSubscribe = this.authorizeSubscribe;

            this.client = mqtt.connect('localhost', {
                port: this.port,
                protocol: 'mqtt',
                username: this.user,
                password: this.password
            });
            this.client.on('connect', () => {
                logger.debug('Connected to mosca broker');
                this.client.subscribe([
                    this.appTopics.request + '+',
                    this.appTopics.control + '+'
                ]);
                this.process();
            })
        });
    }

    process() {
        this.client.on('message', (topic, message) => {
            let topicLevel = topic.split('/');
            logger.debug('RECEIVE request ' + '[' + topicLevel[4] + ']: ' + message);
            if (topicLevel[3] === 'response') {
                this.client.unsubscribe(topic, () => {
                    this.emit('local-response-received', topicLevel[4], message.toString());
                });
            } else {
                this.emit('local-request-received', topicLevel[4], message.toString());
            }
        })
    }

    sendRequest(ID, command) {
        this.client.subscribe(this.appTopics.response + ID, () => {
            this.client.publish(this.hubTopics.request + ID, JSON.stringify(command));
        })
    }

    sendResponse(ID, command) {
        let message = JSON.stringify(command);
        logger.debug('SEND response [' + ID + ']: ' + message);
        this.client.publish(this.hubTopics.response + ID, message);
    }

    sendStatus(ID, command) {
        let message = JSON.stringify(command);
        logger.debug('SEND status [' + ID + ']: ' + message);
        this.client.publish(this.hubTopics.status + ID, message);
    }
}

module.exports = LocalServer;