/**
 * Author: TaiNV
 * Date Modified: 2019/03/31
 * Module: hardware-interface
 * Description: provide hardware's interface including switches and LEDs
 */

// Dependencies
const artik = require('artik-sdk');
const Logger = require('../libraries/system-log');
const EventEmitter = require('events');

const logger = new Logger(__filename);

class HardwareInterface extends EventEmitter {
    constructor() {
        super();
        this.switch = new artik.gpio(30, 'sw403', 'in', 'both', 0);
        this.redLED = new artik.gpio(28, 'RedLED', 'out', 'none', 0);
        this.blueLED = new artik.gpio(38, 'BlueLED', 'out', 'none', 0);
        this.switchTimeCount = 0;
        this.switchTimer = 0;
    };

    start() {
        this.switch.release(); this.switch.request();
        this.redLED.release(); this.redLED.request();
        this.blueLED.release(); this.blueLED.request();
        this._handleSwitchEvent();
    };

    stop() {
        this.switch.release();
        this.redLED.release();
        this.blueLED.release();
    };

    _handleSwitchEvent() {
        this.on('hardware-switch-push', () => {
            logger.debug(name + ' push');
        });
        this.on('hardware-switch-hold', time => {
            logger.debug(name + ' hold for ' + time);
        });
        this.on('hardware-switch-release', time => {
            logger.debug(name + ' release after ' + time);
        });
    };

    process() {
        this.switch.on('changed', value => {
            this.emit('hardware-' + name + '-push');
            if (value === 1) {
                this.switchTimer = setInterval(() => {
                    ++this.switchTimeCount;
                    if (this.switchTimeCount === 3) {
                        this.emit('hardware-switch-hold', 3)
                    } else if (this.switchTimeCount === 5) {
                        this.emit('hardware-switch-hold', 5)
                    }
                }, 1000)
            } else {
                if (this.switchTimer) {
                    clearInterval(this.switchTimer);
                }

                if (this.switchTimeCount < 3) this.switchTimeCount = 0;
                else if (this.switchTimeCount < 5) this.switchTimeCount = 3;
                else this.switchTimeCount = 5;

                this.emit('hardware-switch-release', this.switchTimeCount);
            }
        })
    }
}

// For test
let hardwareInterface = new HardwareInterface();
hardwareInterface.start();
hardwareInterface.process();

process.on('SIGINT', () => {
    hardwareInterface.stop();
});