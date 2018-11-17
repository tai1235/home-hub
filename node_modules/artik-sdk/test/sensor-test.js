/* Global Includes */
var testCase       = require('mocha').describe;
var pre            = require('mocha').before;
var preEach        = require('mocha').beforeEach;
var post           = require('mocha').after;
var postEach       = require('mocha').afterEach;
var assertions     = require('mocha').it;
var assert         = require('chai').assert;
var validator      = require('validator');
var exec           = require('child_process').execSync;
var artik          = require('../src');

var runManualTests = parseInt(process.env.RUN_MANUAL_TESTS);

/* Test Specific Includes */
var module_sensor = artik.sensor();

var acce_sensor;
var humid_sensor;
var photolight_sensor;
var envtemp_sensor;
var proximity_sensor;
var flame_sensor;
var gyro_sensor;
var pressure_sensor;
var hall_sensor;


/* Test Case Module */
testCase('Sensors', function() {

    pre(function() {
        try {
            acce_sensor       = module_sensor.get_accelerometer_sensor(0);
        } catch(e) {
            acce_sensor       = null;
        }
        try {
            humid_sensor      = module_sensor.get_humidity_sensor(0);
        } catch(e) {
            humid_sensor      = null;
        }
        try {
            photolight_sensor = module_sensor.get_light_sensor(0);
        } catch(e) {
            photolight_sensor = null;
        }
        try {
            envtemp_sensor    = module_sensor.get_temperature_sensor(0);
        } catch(e) {
            envtemp_sensor    = null;
        }
        try {
            proximity_sensor  = module_sensor.get_proximity_sensor(0);
        } catch(e) {
            proximity_sensor  = null;
        }
        try {
            flame_sensor      = module_sensor.get_flame_sensor(0);
        } catch(e) {
            flame_sensor      = null;
        }
        try {
            gyro_sensor       = module_sensor.get_gyro_sensor(0);
        } catch(e) {
            gyro_sensor       = null;
        }
        try {
            pressure_sensor   = module_sensor.get_pressure_sensor(0);
        } catch(e) {
            pressure_sensor   = null;
        }
        try {
            hall_sensor       = module_sensor.get_hall_sensor(0);
        } catch(e) {
            hall_sensor       = null;
        }

    });

    testCase('#get_accelerometer_sensor()', function() {


        assertions('#get_speed_x() - get acceleration on X axis', function() {

            if (!acce_sensor)
                this.skip();

            console.log(acce_sensor.get_speed_x());
            assert.notEqual(acce_sensor.get_speed_x(), -1);
        });

        assertions('#get_speed_y() - get acceleration on Y axis', function() {

            if (!acce_sensor)
                this.skip();

            console.log(acce_sensor.get_speed_y());
            assert.notEqual(acce_sensor.get_speed_y(), -1);
        });

        assertions('#get_speed_z() - get acceleration on Z axis', function() {

            if (!acce_sensor)
                this.skip();

            console.log(acce_sensor.get_speed_z());
            assert.notEqual(acce_sensor.get_speed_z(), -1);
        });

    });

    testCase('#get_gyro_sensor()', function() {

        assertions('#get_yaw() - get yaw', function() {
            if (!gyro_sensor)
                this.skip();
            console.log(gyro_sensor.get_yaw());
            assert.notEqual(gyro_sensor.get_yaw(), -1);
        });

        assertions('#get_pitch() - get pitch', function() {
            if (!gyro_sensor)
                this.skip();
            console.log(gyro_sensor.get_pitch());
            assert.notEqual(gyro_sensor.get_pitch(), -1);
        });

        assertions('#get_roll() - get roll', function() {
            if (!gyro_sensor)
                this.skip();
            console.log(gyro_sensor.get_roll());
            assert.notEqual(gyro_sensor.get_roll(), -1);
        });

    });


    testCase('#get_humidity_sensor()', function() {

        assertions('#get_humidity() - returns humidity value', function() {

            if (!humid_sensor)
                this.skip();

            console.log(humid_sensor.get_humidity());
            assert.notEqual(humid_sensor.get_humidity(), -1);
        });
    });

    testCase('#get_light_sensor()', function() {

        assertions('#get_intensity() - returns light sensor intensity', function() {

            if (!photolight_sensor)
                this.skip();

            console.log(photolight_sensor.get_intensity());
            assert.notEqual(photolight_sensor.get_intensity(), -1);
        });

    });

    testCase('#get_temperature_sensor()', function() {

        assertions('#get_celsius() - returns temperature in Celsius', function() {

            if (!envtemp_sensor)
                this.skip();

            console.log(envtemp_sensor.get_celsius());
            assert.notEqual(envtemp_sensor.get_celsius(), -1);
        });

        assertions('#get_fahrenheit() - returns temperature in Fahrenheit', function() {

            if (!envtemp_sensor)
                this.skip();

            console.log(envtemp_sensor.get_fahrenheit());
            assert.notEqual(envtemp_sensor.get_fahrenheit(), -1);
        });
    });

    testCase('#get_proximity_sensor()', function() {

        assertions('#get_presence() - returns presence', function() {

            if (!proximity_sensor || !runManualTests)
                this.skip();

            assert.notEqual(proximity_sensor.get_presence(), -1);
        });

    });

    testCase('#get_flame_sensor()', function() {

        assertions('#get_signals() - returns signals', function() {

            if (!flame_sensor || !runManualTests)
                this.skip();

            assert.notEqual(flame_sensor.get_signals(), -1);
        });

    });

    testCase('#get_pressure_sensor()', function() {

        assertions('#get_pressure() - returns pressure in millibars', function() {

            if (!pressure_sensor)
                this.skip();
            console.log(pressure_sensor.get_pressure());
            assert.isAtLeast(pressure_sensor.get_pressure(), 900);
        });

    });

    testCase('#get_hall_sensor()', function() {

        assertions('#get_detection() - returns 1 if the magnetic field is detected', function() {

            if (!hall_sensor || !runManualTests)
                this.skip();

            assert.equal(hall_sensor.get_detection(), 1);
        });

    });


    post(function() {

    });

});
