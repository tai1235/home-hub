const artik = require('../src');

try {
  var module_sensor = artik.sensor();
  var  list_of_device_sensor = module_sensor.list();
} catch (err) {
  console.log('Ex: ' + err);
  process.exit(-1);
}

for (var i = 0; i < list_of_device_sensor.length; ++i)
    console.log("sensor[%s] of type(%s):index(%d)\n", list_of_device_sensor[i].get_name(), list_of_device_sensor[i].get_type(), list_of_device_sensor[i].get_index());

try {
    var acce_sensor = module_sensor.get_accelerometer_sensor(0);
} catch (err) {
    console.log("Accelerometer sensor : "+err)
}
try {
    var humid_sensor = module_sensor.get_humidity_sensor(0);
} catch (err) {
    console.log("Humidity sensor : "+err)
}
try {
    var photolight_sensor = module_sensor.get_light_sensor(0);
} catch (err) {
    console.log("PhotoLight sensor : "+err)
}
try {
    var envtemp_sensor = module_sensor.get_temperature_sensor(0);
} catch (err) {
    console.log("Environement temperature sensor : "+err)
}
try {
    var proximity_sensor = module_sensor.get_proximity_sensor(0);
} catch (err) {
    console.log("Proximity sensor : "+err)
}
try {
    var flame_sensor = module_sensor.get_flame_sensor(0);
} catch (err) {
    console.log("Flame sensor : "+err)
}

var i = 0;

function push_timeout(a, b) {
   setTimeout(a, b);
}

function my_loop() {

    if (acce_sensor) {
        console.log("Function 'get_speed_x' : %d\n", acce_sensor.get_speed_x());
        console.log("Function 'get_speed_y' : %d\n", acce_sensor.get_speed_y());
        console.log("Function 'get_speed_z' : %d\n", acce_sensor.get_speed_z());
    }

    if (humid_sensor) {
        console.log("Function 'get_humidity' : %d%\n", humid_sensor.get_humidity());
    }

    if (photolight_sensor) {
        console.log("Function 'get_intensity' : %d%\n", photolight_sensor.get_intensity());
    }

    if (envtemp_sensor) {
        console.log("Function 'get_celsius' : %d°C\n", envtemp_sensor.get_celsius());
        console.log("Function 'get_fahrenheit' : %d°F\n", envtemp_sensor.get_fahrenheit());
    }

    if (proximity_sensor) {
        console.log("Function 'get_presence' : %d\n", proximity_sensor.get_presence());
    }

    if (flame_sensor) {
        console.log("Function 'get_signals' : %d\n", flame_sensor.get_signals());
    }

    if (i != -1 && i < 15) {
        push_timeout(my_loop, 1000);
        ++i;
    } else {
        process.exit(0);
    }
}

process.on('SIGINT', function () {
    i = -1;
});

push_timeout(my_loop, 1000);
