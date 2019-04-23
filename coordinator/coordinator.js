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
const LocalServer = require('./local/local-server');
const ServerCommunicator = require('./server/server-communicator');
const HardwareInterface = require('./hardware/hardware-interface');
const helpers = require('../libraries/helpers');
const ZigbeeCommand = require('./zigbee/zigbee').ZigbeeCommand;
const { Commands, CommandData } = require('../controller/command-manager/command-manager');

const logger = new Logger(__filename);

class Coordinator {
    constructor(mac) {
        this.macAddress = mac;
        this.zigbeeGateway = new ZigbeeGateway(config.gatewayId);
        this.deviceManager = new DeviceManager(mac);
        this.databaseManager = new DatabaseManager();
        this.hardwareInterface = new HardwareInterface();
        this.localServer = new LocalServer();
        this.requestHandler = {
            devices: this._handleDeviceRequests,
            rules: this._handleRuleRequests,
            groups: this._handleGroupRequests
        };
        this.responseHandler = {
            devices: this._handleDeviceResponses,
            rules: this._handleRuleResponses,
            groups: this._handleGroupResponses
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
            this.deviceManager.handleDeviceStatus(params.value, params.eui64, params.endpoint);
            // Update reachable status of device to true
            this.databaseManager.handleDeviceStatus(params.eui64);

            // Handle rule input

            // Handle group input

            // Send request to server
        })
    }

    handleDatabaseDeviceAdded() {
        this.databaseManager.on('database-device-added', params => {
            // TODO Send response to server
        })
    }

    handleDatabaseDeviceOffline() {
        this.databaseManager.on('database-device-offline', params => {
            // TODO Send response to server
        })
    }

    handleDatabaseDeviceOnline() {
        this.databaseManager.on('database-device-online', params => {
            // TODO Send response to server
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
            let parsedMessage = JSON.parse(message);
            this.requestHandler[parsedMessage.type](id, parsedMessage.action, parsedMessage.data);
        })
    }


    handleLocalResponse() {
        this.localServer.on('local-response-received', (id, message) => {
            let parsedMessage = JSON.parse(message);
            this.responseHandler[parsedMessage.type](id, parsedMessage.action, parsedMessage.data);
        })
    }

    _handleDeviceRequests(id, action, data) {
        switch (action) {
            case 'search': {
                // let parsedData = CommandData.Devices.CommandSearch.parseData(data);
                // let response;
                // if (parsedData.result !== Commands.StatusCode.SUCCESS) {
                //     response = CommandData.Devices.CommandSearch.createResponse(
                //         parsedData.result, 'Invalid data content', {}
                //     );
                //     this.localServer.sendResponse(id, response);
                //     return;
                // }
                // if (parsedData.data.mac !== this.macAddress) {
                //     response = CommandData.Devices.CommandSearch.createResponse(
                //         Commands.StatusCode.ACTION_FAILED, 'Mac address not recognized', {}
                //     );
                //     this.localServer.sendResponse(response);
                //     return;
                // }
                // if (parsedData.data.act === 1) {
                //     let zigbeeCommand = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.Plugin.PermitJoin);
                //     let duration = 100000;
                //     this.zigbeeGateway.publish(zigbeeCommand);
                //     if (this.deviceSearchTimer) {
                //         clearTimeout(this.deviceSearchTimer);
                //     }
                //     response = CommandData.Devices.CommandSearch.createResponse(
                //         Commands.StatusCode.SUCCESS, 'OK', {...data, duration}
                //     );
                //     this.localServer.sendResponse(id, response);
                //     this.deviceSearchTimer = setTimeout(() => {
                //
                //     }, duration)
                // } else {
                //     if (this.deviceSearchTimer) {
                //         clearTimeout(this.deviceSearchTimer);
                //     }
                //     let zigbeeCommand = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.Plugin.PermitStopJoin);
                //     this.zigbeeGateway.publish(zigbeeCommand);
                //     response = CommandData.Devices.CommandSearch.createResponse(
                //         Commands.StatusCode.SUCCESS, 'OK', {...data, duration: 0}
                //     );
                //     this.localServer.sendResponse(helpers.createRandomString(10), response);
                // }
            } break;
            case '': {

            } break;
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