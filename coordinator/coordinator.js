/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: Handle all events of the system
 */

// Dependencies
const ZigbeeGateway = require('./zigbee/zigbee-gateway');
const Logger = require('../libraries/system-log');
const config = require('../libraries/config');
const DeviceManager = require('../controller/device-manager/device-manager');
const storage = require('node-persist');
const DatabaseManager = require('../controller/database-manager/database-manager');
const Versions = require('../controller/database-manager/versions');
const LocalServer = require('./local/local-server');
const ServerCommunicator = require('./server/server-communicator');
const HardwareInterface = require('./hardware/hardware-interface');
const helpers = require('../libraries/helpers');
const ZigbeeCommand = require('./zigbee/zigbee').ZigbeeCommand;
const { Commands, CommandData } = require('../controller/command-manager/command-manager');

const logger = new Logger(__filename);

class Coordinator {
    constructor(mac) {
        this.zigbeeGateway = new ZigbeeGateway(config.gatewayId);
        this.deviceManager = new DeviceManager(mac);
        this.databaseManager = new DatabaseManager();
        this.hardwareInterface = new HardwareInterface();
        this.localServer = new LocalServer();
        this.requestHandler = {
            devices: this._handleDeviceRequests.bind(this),
            rules: this._handleRuleRequests.bind(this),
            groups: this._handleGroupRequests.bind(this)
        };
        this.responseHandler = {
            devices: this._handleDeviceResponses.bind(this),
            rules: this._handleRuleResponses.bind(this),
            groups: this._handleGroupResponses.bind(this),
            errors: this._handleErrorResponses.bind(this)
        };
    }

    // Device's events from zigbee gateway
    handleZigbeeDeviceJoined() {
        this.zigbeeGateway.on('zigbee-device-joined', params => {
            // Create and store new device to cache
            this.deviceManager.handleDeviceJoined(params.eui64, params.endpoint, params.type);
            // Store device's data to DB
            this.databaseManager.handleDeviceJoined(params.eui64, params.endpoint, params.type);
        })
    }

    handleZigbeeDeviceLeft() {
        this.zigbeeGateway.on('zigbee-device-left', eui64 => {
            // Create and store new device to cache
            this.deviceManager.handleDeviceLeft(eui64);
            // Store device's data to DB
            this.databaseManager.handleDeviceLeft(eui64);
        })
    }

    handleZigbeeDeviceStatus() {
        this.zigbeeGateway.on('zigbee-device-response', params => {
            // Handle device status
            logger.debug(JSON.stringify(params));
            this.deviceManager.handleDeviceStatus(params.value, params.eui64, params.endpoint, (eui64, endpoint, value) => {
                // Send request to server
                let status = CommandData.Devices.CommandStatus.createStatus(eui64, endpoint, value);
                // this.localServer.sendStatus('12345', status); // for testing only
                this.localServer.sendStatus(helpers.createRandomString(10), status);

                // Handle group input

                // Handle rule input

            });
        })
    }

    handleDatabaseDeviceAdded() {
        this.databaseManager.on('database-device-added', params => {
            logger.debug('New device added to DB: ' + JSON.stringify(params));
            let request = CommandData.Devices.CommandAdd.createRequest(params.version, params.data);
            if (request.result === Commands.StatusCode.SUCCESS) {
                logger.debug('Request created successfully, sending to app');
                // this.localServer.sendRequest('12345', request.data); // for testing only
                this.localServer.sendRequest(helpers.createRandomString(10), request);
            } else {
                logger.debug('Request failed to create');
            }
        })
    }

    handleDatabaseDeviceOffline() {
        this.databaseManager.on('database-device-offline', params => {
            // TODO Send response to server
            let offlineDevices = [];
            for (let device of params.data) {
                offlineDevices.push({
                    eui64: device.eui64,
                    endpoint: device.endpoint,
                    status: false
                });
            }
            let request = CommandData.Devices.CommandOnline.createRequest(offlineDevices);
            this.localServer.sendRequest(helpers.createRandomString(10), request);
        })
    }

    handleDatabaseDeviceOnline() {
        this.databaseManager.on('database-device-online', params => {
            // TODO Send response to server
            let onlineDevices = [];
            for (let device of params.data) {
                onlineDevices.push({
                    eui64: device.eui64,
                    endpoint: device.endpoint,
                    status: true
                });
            }
            let request = CommandData.Devices.CommandOnline.createRequest(onlineDevices);
            this.localServer.sendRequest(helpers.createRandomString(10), request);
        })
    }

    handleDatabaseDeviceRemoved() {
        this.databaseManager.on('database-device-removed', params => {
            // TODO Send response to server
        })
    }

    handleHardwareButtonRelease() {
        this.hardwareInterface.on('hardware-button-release', time => {
            switch (time) {
                case 0: {
                    // For future uses
                } break;
                case 3: {
                    // Open network
                    let command = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.Plugin.PermitJoin);
                    this.zigbeeGateway.publish(command);
                } break;
                case 5: {
                    // Reset factory
                    this.deviceManager.reset();
                } break;
            }
        })
    }

    handleLocalRequest() {
        this.localServer.on('local-request-received', (id, message) => {
            let parsedMessage = Commands.Command.parseCommand(message);
            if (parsedMessage.result === Commands.StatusCode.SUCCESS) {
                let parsedCommand = parsedMessage.command;
                this.requestHandler[parsedCommand.type](id, parsedCommand.action, parsedCommand.data);
            } else {
                let response = new Commands.Command('errors', null, {
                    statusCode: parsedMessage.result,
                    statusString: 'Invalid message',
                    returnData: {}
                });
                logger.debug('Message to send: ' + response.fullCommand);
                this.localServer.sendResponse(id, response);
            }
        })
    }


    handleLocalResponse() {
        this.localServer.on('local-response-received', (id, message) => {
            let parsedMessage = Commands.Command.parseCommand(message);
            if (parsedMessage.result === Commands.StatusCode.SUCCESS) {
                let parsedCommand = parsedMessage.command;
                this.requestHandler[parsedCommand.type](id, parsedCommand.action, parsedCommand.data);
            } else {
                this.responseHandler['errors'](id, '', {
                    statusCode: parsedMessage.result,
                    statusString: 'Invalid message',
                    returnData: {}
                })
            }
        })
    }

    _handleDeviceRequests(id, action, data) {
        switch (action) {
            case 'search': {
                // Validate data
                let parsedData = CommandData.Devices.CommandSearch.parseData(data);
                let response;

                // Invalid message
                if (parsedData.result !== Commands.StatusCode.SUCCESS) {
                    response = CommandData.Devices.CommandSearch.createResponse(
                        parsedData.result, 'Invalid data content'
                    );
                    this.localServer.sendResponse(id, response);
                    return;
                }

                // Valid message
                response = CommandData.Devices.CommandSearch.createResponse(
                    Commands.StatusCode.SUCCESS, 'OK'
                );
                this.localServer.sendResponse(id, response);

                if (parsedData.data.act === 1) {
                    // Send Zigbee command
                    let zigbeeCommand = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.Plugin.PermitJoin);
                    this.zigbeeGateway.publish(zigbeeCommand);
                    // Send status message
                    let duration = 100000;
                    let status = CommandData.Devices.CommandSearch.createStatus(1, duration);
                    this.localServer.sendStatus(id, status);

                    if (this.deviceSearchTimer) {
                        clearTimeout(this.deviceSearchTimer);
                    }
                    this.deviceSearchTimer = setTimeout(() => {
                        // Send Zigbee command
                        let zigbeeCommand = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.Plugin.PermitStopJoin);
                        this.zigbeeGateway.publish(zigbeeCommand);
                        // Send status message
                        let status = CommandData.Devices.CommandSearch.createStatus(0, 0);
                        this.localServer.sendStatus(id, status);
                    }, duration)
                } else {
                    if (this.deviceSearchTimer) {
                        clearTimeout(this.deviceSearchTimer);
                    }
                    // Send Zigbee command
                    let zigbeeCommand = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.Plugin.PermitStopJoin);
                    this.zigbeeGateway.publish(zigbeeCommand);
                    // Send status message
                    let status = CommandData.Devices.CommandSearch.createStatus(0, 0);
                    this.localServer.sendStatus(id, status);
                }
            } break;
            case 'control': {
                // Validate data
                let parsedData = CommandData.Devices.CommandControl.parseData(data);
                let response;

                // Invalid message
                if (parsedData.result !== Commands.StatusCode.SUCCESS) {
                    response = CommandData.Devices.CommandControl.createResponse(
                        parsedData.result, 'Invalid data content'
                    );
                    this.localServer.sendResponse(id, response);
                    return;
                }

                // Control devices
                for (let device of parsedData.data) {
                    let eui64 = device.eui64, endpoint = device.endpoint;
                    if (this.deviceManager.getDevice(eui64).getEndpoint(endpoint).permissions.indexOf('w') > -1) {
                        this.deviceManager.getDevice(eui64).getEndpoint(endpoint).setValue(device.value);
                    } else {
                        logger.warn('Read only device could not be controlled');
                    }
                }

                response = CommandData.Devices.CommandControl.createResponse(
                    Commands.StatusCode.SUCCESS, 'OK'
                );
                this.localServer.sendResponse(id, response);
            } break;
            case 'version': {
                let version = Versions.get('devices');
                let response = CommandData.Devices.CommandVersion.createResponse(Commands.StatusCode.SUCCESS, 'OK', version);
                this.localServer.sendResponse(id, response);
            } break;
            case 'sync': {
                this.databaseManager.getAllDevices((e, devices) => {
                    if (e) {
                        let response = new Commands('errors', null, {
                            statusCode: Commands.StatusCode.ACTION_FAILED,
                            statusString: e.message,
                            returnData: {}
                        });
                        this.localServer.sendResponse(id, response);
                        return;
                    }

                    let response = CommandData.Devices.CommandSync.createResponse(Commands.StatusCode.SUCCESS, 'OK', devices);
                    this.localServer.sendResponse(id, response);
                })
            }
        }
    }

    _handleRuleRequests(id, action, data) {

    }

    _handleGroupRequests(id, action, data) {

    }

    _handleDeviceResponses(id, action, data) {

    }

    _handleRuleResponses(id, action, data) {

    }

    _handleGroupResponses(id, action, data) {

    }

    _handleErrorResponses(id, action, data) {

    }

    start() {
        try {
            // Initiate the database manager
            this.databaseManager.start(() => {
                // Initiate the zigbee gateway
                this.zigbeeGateway.start();
                this.zigbeeGateway.onConnect(() => {
                    this.zigbeeGateway.process();
                });
                // Initiate the device manager
                this.databaseManager.getAllDevices((e, devices) => {
                    if (e) throw e;
                    storage.initSync();
                    this.deviceManager.loadDeviceFromDB(devices);
                    this.deviceManager.start();
                });
                // Initiate the rule manager
                // DatabaseManager.getAllRules(rules => {
                //     // Load all rule to cache
                //     logger.debug(rules);
                // });
                // Initiate the group manager
                // DatabaseManager.getAllGroups(groups => {
                //     // Load all groups to cache (?)
                //     logger.debug(groups);
                // });
                // Initiate the hardware interface
                this.hardwareInterface.start();
                this.localServer.start();
            });
        } catch (e) {
            logger.error(e.message);
        }
    }

    process() {
        try {
            // Handle events from Zigbee gateway
            this.handleZigbeeDeviceJoined();
            this.handleZigbeeDeviceLeft();
            this.handleZigbeeDeviceStatus();
            // Handle events from Database
            this.handleDatabaseDeviceAdded();
            this.handleDatabaseDeviceOffline();
            this.handleDatabaseDeviceOnline();
            this.handleDatabaseDeviceRemoved();
            // Handle events from Local
            this.handleLocalRequest();
            this.handleLocalResponse();
            // Handle events from Hardware
            this.handleHardwareButtonRelease();
        } catch (e) {
            logger.error(e.message);
        }

    }
}

module.exports = Coordinator;