const logging = require('./libraries/system-log').create(__filename);

logging.debug('fatal condition');
logging.info('critical condition');
logging.error('error condition');
logging.warn('warning condition');