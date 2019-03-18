/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: device implementation
 */

// Dependencies
const ZigbeeGateway = require('./zigbee-gateway');
const Logger = require('../libraries/system-log');
const DeviceManager = require('../controller/device-manager/device-manager');
const config = require('../config');
const storage = require('node-persist');
const DatabaseManager = require('../controller/db-manager/db-manager');

const logger = new Logger(__filename);

class Coordinator {
    constructor() {
        this.zigbeeGateway = new ZigbeeGateway(config.gatewayId);
        this.deviceManager = new DeviceManager()
    }

    handleDeviceJoined() {
        this.zigbeeGateway.on('device-joined', params => {
            // Create and store new device to cache
            this.deviceManager.handleDeviceJoined(params.eui64, params.endpoint, params.type);

            // Store device's data to DB
            DatabaseManager.handleDeviceJoined(params.eui64, params.endpoint, params.type);

            // Send request to server
        })
    }

    handleDeviceLeft() {
        this.zigbeeGateway.on('device-left', message => {
            // Parse parameter from message

            // Create and store new device to cache
            // this.deviceManager.handleDeviceLeft(eui64);

            // Store device's data to DB
            DatabaseManager.handleDeviceLeft()

            // Send request to server
        })
    }

    handleDeviceStatus() {
        this.zigbeeGateway.on('device-response', params => {
            // Handle device status
            logger.debug(params);
            this.deviceManager.handleDeviceStatus(params.value, params.eui64, params.endpoint);

            // Handle rule input

            // Handle group input

            // Send request to server
        })
    }

    handleDeviceRemove() {
        // Listen event from server

        //
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
        // Initiate the zigbee gateway
        this.zigbeeGateway.getConnectStatus(() => {
            this.zigbeeGateway.process();
        });
        // Initiate the database manager
        DatabaseManager.start(() => {
            DatabaseManager.getAllDevices(devices => {
                logger.debug(devices);
                // Initiate the device manager
                storage.initSync();
                this.deviceManager.start();
                this.deviceManager.loadDeviceFromDB(devices);
            });
            // DatabaseManager.getAllRules(rules => {
            //     // Load all rule to cache
            //     logger.debug(rules);
            // });
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
        this.handleDeviceStatus();
        this.handleDeviceRemove();
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