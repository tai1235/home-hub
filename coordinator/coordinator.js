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

            // Invoke the callback
            // callback();
        })
    }

    handleDeviceLeft() {
        this.zigbeeGateway.on('device-left', message => {
            // Parse parameter from message

            // Create and store new device to cache
            // this.deviceManager.handleDeviceLeft(eui64);

            // Store device's data to DB

            // Invoke the callback
            // callback();
        })
    }

    handleDeviceStatus() {
        this.zigbeeGateway.on('device-response', params => {
            // Handle device status
            this.deviceManager.handleDeviceStatus(params.value, params.eui64, params.endpoint);

            // Handle rule input

            // Handle group input

            // Invoke the callback
            // callback()
        })
    }

    handleRuleAdd() {

    }

    handleRuleRemove() {

    }

    handleRuleActive() {

    }

    handleRuleEnable() {

    }

    handleGroupAdd() {

    }

    handleGroupRemove() {

    }

    handleGroupEnable() {

    }

    start() {
        // Initiate the zigbee gateway
        this.zigbeeGateway.getConnectStatus(() => {
            this.zigbeeGateway.process()
        });
        // Initiate the device manager
        storage.initSync();
        this.deviceManager.start();
    }

    process() {
        // Handle device's event
        this.handleDeviceJoined();
        this.handleDeviceLeft();
        this.handleDeviceStatus();
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