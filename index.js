const log = require('./lib/util/system-log').create(__filename);

log.debug('fatal condition');
log.info('critical condition');
log.error('error condition');
log.warn('warning condition');