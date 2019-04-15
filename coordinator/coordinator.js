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
const ZigbeeCommand = require('./zigbee/zigbee').ZigbeeCommand;

const logger = new Logger(__filename);

class Coordinator {
    constructor(mac) {
        this.zigbeeGateway = new ZigbeeGateway(config.gatewayId);
        this.deviceManager = new DeviceManager(mac);
        this.databaseManager = new DatabaseManager();
        this.hardwareInterface = new HardwareInterface();
        this.localServer = new LocalServer();
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

    // Device's events from server
    handleServerDeviceRemove() {
        // Listen event from server

        // Handle event
        // this.deviceManager.handleDeviceRemove(eui64)
    }

    handleServerDeviceControl() {
        // Listen event from server

        // Handle device control
        // this.deviceManager.handleDeviceControl(params.value, params.eui64, params.endpoint);
    }

    handleServerRuleAdd() {
        // Handle rule add

        // Store rule's data
        this.databaseManager.handleRuleAdd();

        // Send response to server
    }

    handleServerRuleRemove() {
        // Handle rule remove

        // Update database
        this.databaseManager.handleRuleRemove();

        // Send response to server
    }

    handleServerRuleActive() {
        // Handle rule active

        // Update database
        this.databaseManager.handleRuleActive()

        // Send response to server
    }

    handleServerRuleEnable() {
        // Handle rule enable

        // Update database
        this.databaseManager.handleRuleEnable()

        // Send response to server
    }

    handleServerGroupAdd() {
        // Handle group add

        // Update database
        this.databaseManager.handleGroupAdd()

        // Send response to server
    }

    handleServerGroupRemove() {
        // Handle group remove

        // Update database
        this.databaseManager.handleGroupRemove()

        // Send response to server
    }

    handleServerGroupEnable() {
        // Handle group enable

        // Update database
        this.databaseManager.handleGroupEnable()

        // Send response to server
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
            // Handle events from Server
            this.handleServerDeviceRemove();
            this.handleServerDeviceControl();
            this.handleServerRuleAdd();
            this.handleServerRuleRemove();
            this.handleServerRuleEnable();
            this.handleServerRuleActive();
            this.handleServerGroupAdd();
            this.handleServerGroupRemove();
            this.handleServerGroupEnable();
            // Handle events from Hardware
            this.handleHardwareButtonRelease();
        } catch (e) {
            logger.error(e.message);
        }

    }
}

module.exports = Coordinator;