/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: Handle all events of the system
 */

// Dependencies
const ZigbeeGateway = require('./zigbee-gateway');
const Logger = require('../libraries/system-log');
const config = require('../config');
const DeviceManager = require('../controller/device-manager/device-manager');
const storage = require('node-persist');
const DatabaseManager = require('../controller/database-manager/database-manager');
const ServerCommunicator = require('../coordinator/server-communicator');

const logger = new Logger(__filename);

class Coordinator {
    constructor() {
        this.zigbeeGateway = new ZigbeeGateway(config.gatewayId);
        this.deviceManager = new DeviceManager();
        // this.serverCommunicator = new ServerCommunicator({
        //     server: config.server,
        //     cloudMQTT: config.cloudMQTT
        // });
    }

    // Device's events from zigbee gateway
    handleDeviceJoined() {
        this.zigbeeGateway.on('device-joined', params => {
            logger.info(JSON.stringify({
                eui64: params.eui64,
                endpoint: params.endpoint,
                type: params.type
            }));
            // Create and store new device to cache
            this.deviceManager.handleDeviceJoined(params.eui64, params.endpoint, params.type);
            // Store device's data to DB
            DatabaseManager.handleDeviceJoined(params.eui64, params.endpoint, params.type);
            // Send request to server
        })
    }

    handleDeviceLeft() {
        this.zigbeeGateway.on('device-left', eui64 => {
            // Create and store new device to cache
            this.deviceManager.handleDeviceLeft(eui64);
            // Store device's data to DB
            DatabaseManager.handleDeviceLeft(eui64)

            // Send request to server
        })
    }

    handleDeviceStatus() {
        this.zigbeeGateway.on('device-response', params => {
            // Handle device status
            logger.debug(JSON.stringify(params));
            this.deviceManager.handleDeviceStatus(params.value, params.eui64, params.endpoint);
            // Update reachable status of device to true
            DatabaseManager.handleDeviceStatus(params.eui64);

            // Handle rule input

            // Handle group input

            // Send request to server
        })
    }

    // Device's events from server
    handleDeviceRemove() {
        // Listen event from server

        // Handle event
        this.deviceManager.handleDeviceRemove(eui64)
    }

    handleDeviceControl() {
        // Listen event from server

        // Handle device control
        this.deviceManager.handleDeviceControl(params.value, params.eui64, params.endpoint);
    }

    handleRuleAdd() {
        // Handle rule add

        // Store rule's data
        DatabaseManager.handleRuleAdd();

        // Send response to server
    }

    handleRuleRemove() {
        // Handle rule remove

        // Update database
        DatabaseManager.handleRuleRemove();

        // Send response to server
    }

    handleRuleActive() {
        // Handle rule active

        // Update database
        DatabaseManager.handleRuleActive()

        // Send response to server
    }

    handleRuleEnable() {
        // Handle rule enable

        // Update database
        DatabaseManager.handleRuleEnable()

        // Send response to server
    }

    handleGroupAdd() {
        // Handle group add

        // Update database
        DatabaseManager.handleGroupAdd()

        // Send response to server
    }

    handleGroupRemove() {
        // Handle group remove

        // Update database
        DatabaseManager.handleGroupRemove()

        // Send response to server
    }

    handleGroupEnable() {
        // Handle group enable

        // Update database
        DatabaseManager.handleGroupEnable()

        // Send response to server
    }

    start() {
        // Initiate the database manager
        DatabaseManager.start(() => {
            // Initiate the zigbee gateway
            this.zigbeeGateway.start();
            this.zigbeeGateway.onConnect(() => {
                this.zigbeeGateway.process();
            });
            // Initiate the device manager
            DatabaseManager.getAllDevices(devices => {
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
        });
    }

    process() {
        // Handle device's event
        this.handleDeviceJoined();
        this.handleDeviceLeft();
        this.handleDeviceRemove();
        this.handleDeviceStatus();
        this.handleDeviceControl();
        // Handle rule's event
        this.handleRuleAdd();
        this.handleRuleRemove();
        this.handleRuleEnable();
        this.handleRuleActive();
        // Handle group's event
        this.handleGroupAdd();
        this.handleGroupRemove();
        this.handleGroupEnable();
    }
}

module.exports = Coordinator;