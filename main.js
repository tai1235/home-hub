const fs = require('fs');
const Log = require('./lib/system-log');

const log = new Log(fs.createWriteStream('/tmp/log.txt'));
