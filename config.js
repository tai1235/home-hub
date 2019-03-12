/**
 * Author: TaiNV
 * Date Modified: 2019/01/19
 * Module: system-log
 * Description: configuration for the project
 */

// Container for all environments
let environments = {};

// Staging (default) environment
environments.staging = {
    hostName: 'http://e835f1e4.ngrok.io',
    httpPort: 3000,
    httpsPort: 3001,
    envName: 'staging',
    hashingSecret: 'alpaca',
    logPath: '/home/tainv/home-hub/log.txt',
    gatewayId: '000B57FFFE572517'
};

// Production environment
environments.production = {
    hostName: 'http://e835f1e4.ngrok.io',
    httpPort: 5000,
    httpsPort: 5001,
    envName: 'production',
    hashingSecret: 'wolverine',
    logPath: '/home/tainv/home-hub/log.txt',
    gatewayId: '000B57FFFE572517'
};

// Determine which environment was passed as a command-line argument
let currentEnvironment = typeof (process.env.NODE_ENV) == 'string' ? process.env.NODE_ENV.toLowerCase() : '';

// Check that the current environment is one of the environments above, if not default to staging
let environmentToExport = typeof (environments[currentEnvironment]) == 'object' ?
    environments[currentEnvironment] : environments.staging;

// Export the module
module.exports = environmentToExport;