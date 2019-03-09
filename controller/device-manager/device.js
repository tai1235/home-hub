/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: device implementation
 */

// Dependencies
let Accessory = require('../../hap').Accessory
let Service = require('../../hap').Service
let Characteristic = require('../../hap').Characteristic
let uuid = require('../../hap').uuid;
let Logger = require('../../libraries/system-log')
// let MqttClient = require('../../coordinator/')

let logger = new Logger(__filename)

// Device class
class Device extends Accessory {
    constructor(eui64, name, type, endpoint, serialNumber = '', manufacturer = '', model = '') {
        super(name, uuid.generate('hap-nodejs:accessories:' + type + name))
        this.eui64 = eui64
        this.type = type
        this.status = []
        if (serialNumber)
            this.getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.SerialNumber, serialNumber)
        if (manufacturer)
            this.getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.Manufacturer, manufacturer)
        if (model)
            this.getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.Model, model)
        // Add listener to identify event
        this.on('indentify', (paired, callback) => {
            if (paired)
                logger.debug('Deivce ' + this.eui64 + ' has been identified')
            callback()
        })
        // Add services due to device's type and number of endpoint
        if (type = 'switch') {
            for (let i = 0; i < endpoint; i++) {
                let status = { on: false }
                this.status.push(status)
                logger.debug("Adding services")
                let endpointService = new Service.Switch('Switch_' + (i + 1), i)
                this.addService(endpointService)
                    .getCharacteristic(Characteristic.On)
                    .on('set', (value, callback) => {
                        // _sendZigbeeCommand(ZigbeeCommand.OnOff, {eui64: this.eui64, endpoint: i, value: value}, postTimeDelay) // Need definition
                        logger.info("Set characteristic")
                        this.status[i].on = value
                        callback()
                    })
                    .on('get', callback => {
                        logger.info("Get characteristic")
                        callback(null, this.status[i].on)
                    })
            }
        } else if (type = 'light') {
            for (let i = 0; i < endpoint; i++) {
                let status = { on: false, brigthness: 0 }
                this.status.push(status)
                let endpointService = new Service.Light('Light_' + (i + 1), i)
                this.addService(endpointService)
                    .getCharacteristic(Characteristic.On)
                    .on('set', (value, callback) => {
                        _sendZigbeeCommand(ZigbeeCommand.OnOff, {eui64: this.eui64, endpoint: i, value: value}, postTimeDelay) // Need definition
                        this.status[i].on = value
                        callback()
                    })
                    .on('get', callback => {
                        callback(null, this.status[i].on)
                    })
                this.getService(endpointService)
                    .addCharacteristic(Characteristic.Brightness)
                    .on('set', (value, callback) => {
                        _sendZigbeeCommand(ZigbeeCommand.Level, {eui64: this.eui64, endpoint: i, value: value}, postTimeDelay) // Need definition
                        this.status[i].brightness = value
                        callback()
                    })
                    .on('get', callback => {
                        callback(null, this.status[i].brightness)
                    })
            }
        }
    }

    
}

module.exports = Device