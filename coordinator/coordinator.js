/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: device implementation
 */

// Dependencies
const ZigbeeGateway = require('./zigbee-gateway');
const EventEmitter = require('events');
const Logger = require('../libraries/system-log');
const DeviceManager = require('../controller/device-manager/device-manager');
const Device = require('../controller/device-manager/device');
const config = require('../config');

const logger = new Logger(__filename);

class Coordinator {
    constructor() {
        this.zigbeeGateway = new ZigbeeGateway(config.gatewayId);
        this.zigbeeGateway.getConnectStatus(() => {
            this.zigbeeGateway.process()
        });
        this.deviceManager = new DeviceManager()
    }

    handleDeviceJoined(callback) {
        this.zigbeeGateway.on('device-joined', message => {
            // Parse parameter from message

            // Create and store new device to cache
            this.deviceManager.handleDeviceJoined(eui64, endpoint, type);

            // Store device's data to DB

            // Invoke the callback
            callback();
        })
    }

    handleDeviceLeft(callback) {
        this.zigbeeGateway.on('device-left', message => {
            // Parse parameter from message

            // Create and store new device to cache
            this.deviceManager.handleDeviceLeft(eui64);

            // Store device's data to DB

            // Invoke the callback
            callback();
        })
    }

    handleDeviceStatus(callback) {
        this.zigbeeGateway.on('device-status', message => {
            // Handle device status

            // Handle rule input

            // Handle group input

            // Invoke the callback
            callback()
        })
    }

    handleRuleAdd(callback) {

    }

    handleRuleRemove(callback) {

    }

    handleRuleActive(callback) {

    }

    handleRuleEnable(callback) {

    }

    handleGroupAdd(callback) {

    }

    handleGroupRemove(callback) {

    }

    handleGroupEnable(callback) {

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

module.exports = Coordinator