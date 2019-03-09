/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: device implementation
 */

// Dependencies
const ZigbeeGateway = require('./zigbee-gateway')
const EventEmitter = require('events')
const Logger = require('../libraries/system-log')
const DeviceManager = require('../controller/device-manager/device-manager')
const Device = require('../controller/device-manager/device')

const logger = new Logger(__filename)

class Coordinator extends EventEmitter {
    constructor() {
        this.zigbeeGateway = new ZigbeeGateway()
        this.mqttClient.getConnectStatus(() => {
            this.mqttClient.getMessage()
        })
        this.deviceManager = new DeviceManager()
    }

    handleDeviceJoin(callback) {
        this.zigbeeGateway.on('device-join', message => {
            // Handle device join
            // Get device's information from message
            let strMessage = JSON.parse(message);
            let eui64 = strMessage.endpoint.eiu64 // TODO
            let device = new Device(eui64, name, type, endpoint, serialNumber, manufacturer, model)
            this.deviceManager.addDevice(device)
            // Update database

            // Invoke the callback
            callback(device)
        })
    }

    handleDeviceLeft(callback) {
        this.zigbeeGateway.on('device-left', message => {
            // Handle device left

            // Invoke the callback
            callback()
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

    process() {
        this.handleDeviceJoinMessage()
        this.handleDeviceLeftMessage()
        this.handleDeviceStatusMessage()
    }
}

module.exports = Coordinator