/**
 * Author: TaiNV
 * Date Created: 2019/03/09
 * Module: http-client
 * Description: Communication with Server using HTTP
 */

// Dependencies
let http = require('http');
let https = require('https');
let config = require('../config');
let EventEmitter = require('events');

class HTTPClient extends EventEmitter {
    constructor(hostname, port) {
        super();
        this.options = {

        }
    }
}

