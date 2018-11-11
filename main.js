const fs = require('fs');
const Log = require('./lib/system-log');

const log = new Log(fs.createWriteStream('/tmp/log.txt'));
// const log = new Log();

log.fatal('fatal condition');
log.critical('critical condition');
log.error('error condition');
log.warning('warning condition');
log.info('sending email');
log.debug('preparing email');

