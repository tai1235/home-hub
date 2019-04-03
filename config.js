/**
 * Author: TaiNV
 * Date Modified: 2019/01/19
 * Module: config
 * Description: configuration for the project
 */

// Container for all environments
let environments = {
    // Staging (default) environment
    staging: {
        envName: 'staging',
        server: {
            host: 'http://e835f1e4.ngrok.io',
            authentication: {
                key: '',
            }
        },
        hashingSecret: 'alpaca',
        logPath:  __dirname + '/log.txt',
        gatewayId: '000B57FFFE572517',
        cloudMQTT: {
            host: 'm24.cloudmqtt.com',
            port: 16032,
            authentication: {
                username: 'bkxttvrf',
                password: 'x1q_MUb27bLz'
            }
        },
        runningMode: 0,
        resourceID: 'home-hub_1_1'
    },
    // Production environment
    production: {
        envName: 'production',
        server: {
            host: 'http://e835f1e4.ngrok.io',
            authentication: {
                key: '',
            }
        },
        hashingSecret: 'alpaca',
        logPath:  __dirname + '/log.txt',
        gatewayId: '000B57FFFE572517',
        cloudMQTT: {
            host: 'm24.cloudmqtt.com',
            port: 16032,
            authentication: {
                username: 'bkxttvrf',
                password: 'x1q_MUb27bLz'
            }
        },
        runningMode: 0,
        resourceID: 'home-hub_1_1'
    }
};

// Determine which environment was passed as a command-line argument
let currentEnvironment = typeof (process.env.NODE_ENV) == 'string' ? process.env.NODE_ENV.toLowerCase() : '';

// Check that the current environment is one of the environments above, if not default to staging
let environmentToExport = typeof (environments[currentEnvironment]) == 'object' ?
    environments[currentEnvironment] : environments.staging;

// Export the module
module.exports = environmentToExport;