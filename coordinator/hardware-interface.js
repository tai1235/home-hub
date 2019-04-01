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
        this.switch = {
            sw403: new artik.gpio(30, 'sw403', 'in', 'both', 0),
            sw404: new artik.gpio(32, 'sw404', 'in', 'both', 0)
        };
        this.led =  {
            redLED: new artik.gpio(28, 'RedLED', 'out', 'none', 0),
            blueLED: new artik.gpio(38, 'BlueLED', 'out', 'none', 0)
        };
        this.switchTimeCount = { sw403: 0, sw404: 0 };
        this.switchTimer = { sw403: 0, sw404: 0 };
    };

    start() {
        this.sw403.release(); this.sw403.request();
        this.sw404.release(); this.sw404.request();
        this.redLED.release(); this.redLED.request();
        this.blueLED.release(); this.blueLED.request();
        this._handleSwitchEvent('sw403');
        this._handleSwitchEvent('sw404');
    };

    stop() {
        this.sw403.release();
        this.sw404.release();
        this.redLED.release();
        this.blueLED.release();
    };

    _handleSwitchEvent(name) {
        this.on('hardware-' + name + '-push', () => {

        });
        this.on('hardware-' + name + '-hold', time => {

        });
        this.on('hardware-' + name + '-release', time => {

        });
    };

    process() {
        this.switch[name].on('changed', value => {
            this.emit('hardware-' + name + '-push');
            if (value === 1) {
                this.switchTimer[name] = setInterval(() => {
                    ++this.switchTimeCount[name];
                    if (this.switchTimeCount[name] === 3) {
                        this.emit('hardware-' + name + '-hold', 3)
                    } else if (this.switchTimeCount[name] === 5) {
                        this.emit('hardware-' + name + '-hold', 5)
                    }
                }, 1000)
            } else {
                if (this.switchTimer[name]) {
                    clearInterval(this.switchTimer[name]);
                }

                if (this.switchTimeCount[name] < 3) this.switchTimeCount[name] = 0;
                else if (this.switchTimeCount[name] < 5) this.switchTimeCount[name] = 3;
                else this.switchTimeCount[name] = 5;

                this.emit('hardware-' + name + '-release', this.switchTimeCount[name]);
            }
        })
    }
}