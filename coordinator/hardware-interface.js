/**
 * Author: TaiNV
 * Date Modified: 2019/03/31
 * Module: hardware-interface
 * Description: provide hardware's interface including button and LEDs
 */

// Dependencies
const artik = require('artik-sdk');
const Logger = require('../libraries/system-log');
const EventEmitter = require('events');
const debug = require('debug')('HardwareInterface');

const logger = new Logger(__filename);

class HardwareInterface extends EventEmitter {
    constructor() {
        super();
        this.button = new artik.gpio(30, 'sw403', 'in', 'both', 0);
        this.redLED = new artik.gpio(28, 'RedLED', 'out', 'none', 0);
        this.blueLED = new artik.gpio(38, 'BlueLED', 'out', 'none', 0);
        this.buttonTimeCount = 0;
        this.buttonTimer = 0;
    };

    start() {
        let result = this.button.request();
        debug('Button request return code ' + result);
        if (result !== 0) {
            debug('Release button');
            if (this.button.release() === 0) {
                debug('Release button success');
                result = this.button.request();
                debug('Button request return code ' + result);
            } else {
                debug('Release button fail');
            }
        }
        result = this.redLED.request();
        debug('RedLED request return code ' + result);
        if (result !== 0) {
            debug('Release RedLED');
            if (this.redLED.release() === 0) {
                debug('Release RedLED success');
                result = this.redLED.request();
                debug('RedLED request return code ' + result);
            } else {
                debug('Release RedLED fail');
            }
        }
        result = this.blueLED.request();
        debug('BlueLED request return code ' + result);
        if (result !== 0) {
            debug('Release BlueLED');
            if (this.blueLED.release() === 0) {
                debug('Release BlueLED success');
                result = this.blueLED.request();
                debug('BlueLED request return code ' + result);
            } else {
                debug('Release BlueLED fail');
            }
        }
        this._handleSwitchEvent();
        this.process();
    };

    stop() {
        this.button.release();
        this.redLED.release();
        this.blueLED.release();
    };

    _handleSwitchEvent() {
        this.on('hardware-button-push', () => {
            logger.debug('button push');
        });
        this.on('hardware-button-hold-3', () => {
            logger.debug('button hold for 3s');
            this.redLED.write(1);
        });
        this.on('hardware-button-hold-5', () => {
            logger.debug('button hold for 5s');
            this.blueLED.write(1);
        });
        this.on('hardware-button-release', time => {
            logger.debug('button release after ' + time + 's');
            this.buttonTimeCount = 0;
            this.blueLED.write(0);
            this.redLED.write(0);
        });
    };

    process() {
        this.button.on('changed', value => {
            if (value === '0') {
                this.emit('hardware-button-push');
                this.buttonTimer = setInterval(() => {
                    ++this.buttonTimeCount;
                    if (this.buttonTimeCount === 3) {
                        this.emit('hardware-button-hold-3')
                    } else if (this.buttonTimeCount === 5) {
                        this.emit('hardware-button-hold-5')
                    }
                }, 1000);
            } else {
                if (this.buttonTimer) {
                    clearInterval(this.buttonTimer);
                }

                if (this.buttonTimeCount < 3) this.buttonTimeCount = 0;
                else if (this.buttonTimeCount < 5) this.buttonTimeCount = 3;
                else this.buttonTimeCount = 5;

                this.emit('hardware-button-release', this.buttonTimeCount);
            }
        })
    }
}

module.exports = HardwareInterface;