var util = require('util')
var EventEmitter = require('events').EventEmitter
var zigbee = require('../build/Release/artik-sdk.node').zigbee;
/**
 * ZigBee module
 *
 * Network topology:
 *
 *   Coordinator --- Router --- End-Node
 *          \            \--- Router --- End-Node
 *           \--- End-Node
 *
 * @section Usage
 * @code
 *   var Zigbee = require('zigbee').Zigbee
 *   var Light = require('zigbee').ZigbeeDevices.ONOFF_LIGHT
 *
 *   // create zigbee handle
 *   var myzigbee = new Zigbee()
 *
 *   // create ONOFF Light device handle with endpoint ID(1)
 *   var mydevice = new Light(myzigbee, 1)
 *
 *   myzigbee.initialize()
 *   myzigbee.start()
 *   myzigbee.join() or .form() or nothing(already network joined)
 *   myzigbee.on('network_notification', function (event) { ... })
 *   ...
 *
 *   var status = mydevice.onoff_get_value()
 *   console.log('status =', status)
 * @endcode
 *
 * @section Events
 * Zigbee events
 *
 * @event receive_command
 * @param {Object} {
 *     is_global_command: {Number},
 *     endpoint_id: {Number},
 *     cluster_id: {Number},
 *     command_id: {Number},
 *     payload: {String},
 *     source_device_id: {Number},
 *     source_endpoint_id: {Number}
 *   }
 *
 * @event attribute_change
 * @param {Object} {
 *     attr: {String},
 *     endpoint_id: {Number}
 *   }
 *
 * @event reporting_configure
 * @param {Object} {
 *     used: {Number},
 *     endpoint_id: {Number},
 *     cluster_id: {Number},
 *     attribute_id: {Number},
 *     is_server: {Number},
 *     reported: {
 *       min_interval: {Number},
 *       max_interval: {Number},
 *       reportable_change: {Number}
 *     }
 *   }
 *
 * @event report_attribute
 * @param {Object} {
 *     attr: {String},
 *     value: {Number}
 *   }
 *
 * @event identify_feedback_start
 * @param {Object} {
 *     endpoint_id: {Number},
 *     duration: {Number}
 *   }
 *
 * @event identify_feedback_stop
 * @param {Object} {
 *     endpoint_id: {Number},
 *     duration: {Number}
 *   }
 *
 * @event network_notification
 * @param {Object} {
 *     status: {String} ('join', 'leave', 'find_form', 'find_form_failed',
 *             'find_join', 'find_join_failed')
 *   }
 *
 * @event network_find
 * @param {Object} {
 *     status: {String} ('found', 'finished'),
 *     channel: {Number},
 *     tx_power: {Number},
 *     pan_id: {Number}
 *   }
 *
 * @event device_discover
 * @param {Object} {
 *     status: {String} ('start', 'found', 'in_progress', 'done', 'no_device', 'error',
 *             'changed', 'lost'),
 *     device: {Object} (Deviceinfo object)
 *   }
 *
 * @event device_left
 * @param none
 *
 * @event broadcast_identify_query_response
 * @param {Object} {
 *     node_id: {Number},
 *     endpoint_id: {Number},
 *     timeout: {Number}
 *   }
 *
 * @event groups_info
 * @param {Object} {
 *     command: {Number},
 *     group_id: {Number},
 *     endpoint_id: {Number}
 *   }
 *
 * @event commissioning_status
 * @param {Object} {
 *     status: {String} ('error_in_progress', 'network_steering_form',
 *             'network_steering_success', 'network_steering_failed',
 *             'network_steering', 'initiator_success', 'initiator_failed',
 *             'initiator_stop', 'target_success', 'target_failed', target_stop')
 *   }
 *
 * @event commissioning_target_info
 * @param {Object} {
 *     node_id: {Number},
 *     endpoint_id: {Number}
 *   }
 *
 * @event commissioning_bound_info
 * @param {Object} {
 *     node_id: {Number},
 *     endpoint_id: {Number},
 *     cluster_id: {Number}
 *   }
 *
 * @event ieee_addr
 * @param {Object} {
 *     status: {String}, ('success', 'error')
 *     node_id: {Number},
 *     eui64: {String}
 *   }
 *
 * @event simple_desc
 * @param {Object} {
 *     status: {String}, ('success', 'error')
 *     target_node_id: {Number},
 *     target_endpoint: {Number},
 *     server_cluster: {Array},
 *     client_cluster: {Array}
 *   }
 *
 * @event match_desc
 * @param {Object} {
 *     status: {String}, ('success', 'received', 'error')
 *     node_id: {Number},
 *     endpoints: {Array}
 *   }
 *
 * @event basic_reset_to_factory
 * @param {Object} {
 *     endpoint_id: {Number}
 *   }
 *
 * @section Object types
 * - Device object
 *   {
 *     device_id: {Number},
 *     profile_id: {Number},
 *     handle: {Number}
 *   }
 *
 * - Deviceinfo object
 *   {
 *     eui64: {String},
 *     node_id: {Number},
 *     endpoints: {Array} (Endpoint object)
 *   }
 *
 * - Endpoint object
 *   {
 *     endpoint_id: {Number},
 *     node_id: {Number},
 *     server_cluster: {Array} ([ 1, 2, 3,-1,-1,-1,-1,-1,-1]),
 *     client_cluster: {Array} ([ 1, 2,-1,-1,-1,-1,-1,-1,-1])
 *   }
 */
function Zigbee (opts) {
  EventEmitter.call(this)

  this.api = new zigbee()
  setImmediate(function (self) {
    self.emit('started')
  }, this)
}

util.inherits(Zigbee, EventEmitter)

/**
 * Initialize ZigBee API (Setup communication with zigbee daemon)
 */
Zigbee.prototype.initialize = function () {
  var _ = this

  this.api.initialize(function (data) {
    var event = JSON.parse(data)
    _.emit(event.type, event)
  })
}

/**
 * Start request to zigbee daemon
 *
 * @return {String} current network state.
 *                 'no_network', 'joining_network', 'joined_network',
 *                 'joined_network_no_parent', 'leaving_network'
 */
Zigbee.prototype.network_start = function () {
  return this.api.network_start()
}

/**
 * Leave from the joined network
 */
Zigbee.prototype.network_leave = function () {
  this.api.network_leave()
}

/**
 * Coordinator - Form a new network as a coordinator.
 *
 * @param {Object} opts Manual options for form. tx_power: -43 ~ 8
 */
Zigbee.prototype.network_form = function (opts) {
  if (opts) {
    this.api.network_form_manually(opts.channel, opts.tx_power, opts.pan_id)
  } else {
    this.api.network_form()
  }
}

/**
 * Coordinator - Permit joining to the formed network from other nodes
 *
 * @param {Number} duration Seconds value to permit joining
 */
Zigbee.prototype.network_permitjoin = function (duration) {
  this.api.network_permitjoin(duration)
}

/**
 * Join to the existing network by other coordinator automatically.
 * Router/End-node
 *
 * @param {Object} opts Manual options for network. tx_power: -43 ~ 8
 */
Zigbee.prototype.network_join = function (opts) {
  if (opts) {
    this.api.network_join_manually(opts.channel, opts.tx_power, opts.pan_id)
  } else {
    this.api.network_join()
  }
}

/**
 * Scan existing networks
 */
Zigbee.prototype.network_find = function () {
  this.api.network_find()
}

/**
 * Stop the network scanning.
 *
 * When 'network form', 'network join' and 'network find' are
 * called, network scanning is conducted in zigbeed side, this
 * api is provided to stop network scanning.
 */
Zigbee.prototype.network_stop_scan = function () {
  this.api.network_stop_scan()
}

/**
 * Reset local device attribute, binding list and initialize network.
 */
Zigbee.prototype.reset_local = function () {
  this.api.reset_local()
}

/**
 * Get current network status
 *
 * @return {String} current network state.
 *                  'no_network', 'joining_network', 'joined_network',
 *                  'joined_network_no_parent', 'leaving_network'
 */
Zigbee.prototype.network_request_my_network_status = function () {
  return this.api.network_request_my_network_status()
}

/**
 * Get my node type
 *
 * @return {String} Node type.
 *                  'coordinator', 'router', 'end_device', 'sleepy_end_device'
 */
Zigbee.prototype.device_request_my_node_type = function () {
  return this.api.device_request_my_node_type()
}

/**
 * Get endpoint list to filtered by cluster ID
 *
 * @return {Array} Array of endpoint object.
 */
Zigbee.prototype.device_find_by_cluster = function (clusterId) {
  return JSON.parse(this.api.device_find_by_cluster(clusterId))
}

/**
 * Request device/service discovery
 *
 * Device/service discovery is a cyclic call, and the default
 * cyclic duration is 1 minute, for the cyclic duration setting,
 * please check api 'set_discover_cycle_time'.
 * If current api 'device_discover' is called, the discovery is
 * triggered immediately.
 *
 * 'device_discover' event will be invoked.
 */
Zigbee.prototype.device_discover = function () {
  this.api.device_discover()
}

/**
 * Set the cyclic duaration of device discovery cycle
 *
 * @param {Number} cycleMinutes The cyclic duaration, in minutes,
 */
Zigbee.prototype.set_discover_cycle_time = function (cycleMinutes) {
  return this.api.set_discover_cycle_time(cycleMinutes)
}

/**
 * Get discovered device list
 *
 * @return {Array} Array of Deviceinfo object.
 */
Zigbee.prototype.get_discovered_device_list = function () {
  return JSON.parse(this.api.get_discovered_device_list())
}

/**
 * Get my local device list
 *
 * @return {Array} Array of Device object.
 */
Zigbee.prototype.get_local_device_list = function () {
  return JSON.parse(this.api.get_local_device_list())
}

module.exports.Zigbee = Zigbee

/**
 * ZigBee Device APIs
 *
 * - groups_get_local_name_support()
 *   Get local attribute of "name support" of cluster "Groups".
 *   @return {Boolean} true(supported), false(not supported)
 *
 * - groups_set_local_name_support(support)
 *   Set local attribute of "name support" of cluster "Groups".
 *   @param {Boolean} support flag
 *
 * - ezmode_commissioning_target_start()
 *   Start ezmode commissioning on some endpoint as target
 *
 * - ezmode_commissioning_target_stop()
 *   Stop ezmode commissioning on some endpoint as target
 *
 * - ezmode_commissioning_initiator_start()
 *   Start ezmode commissioning on some endpoint as initiator
 *
 * - ezmode_commissioning_initiator_stop()
 *   Stop ezmode commissioning on some endpoint as initiator
 *
 * - identify_request()
 *   Send command "Identify".
 *
 * - identify_get_remaining_time()
 *   Send command "Identify Query", to get remote identify time.
 *   @return {Number} Remaining seconds of identifying.
 *
 * - onoff_command(endpoint, command)
 *   Send command to control remote on/off.
 *   @param {Object} endpoint
 *   @param {String} command 'on', 'off', 'toggle'
 *
 * - onoff_get_value()
 *   Get attribute of "on/off" of cluster "On/off".
 *   @return {String} 'on' or 'off'
 *
 * - level_control_request(endpoint, levelCommand)
 *   Send commands about level control, to control remote level.
 *   @param {Object} endpoint
 *   @param {Object} levelCommand {
 *     type: {String}, ('stop', 'moveup', 'movedown', 'moveto', 'stepup', 'stepdown')
 *     value: {Number},
 *     transition_time: {Number}, (optional. default 0, tenths of a second)
 *     auto_onoff: {Boolean} (optional. default false)
 *   }
 *
 * - level_control_get_value()
 *   Get attribute of "current level" of cluster "Levle Control".
 *   @return {Number} Current level of this device.
 *
 * - illum_set_measured_value_range(min, max)
 *   Set the range of illuminance in measured value(1 to 65534(0xFFFE))
 *   Max value shall be greater than min value.
 *   @param {Number} min Min measured value
 *   @param {Number} max Max measured value
 *
 * - illum_set_measured_value(value)
 *   Set the measured illuminance value.
 *   The value should calculated as logarithm with base 10
 *     10,000 x log10(Illuminance) + 1
 *   The possible illuminance are in the range 1 lx to 3.576 x 10^6 lx,
 *   corresponding to values of 1 to 0xFFFE.
 *   e.g. To set the sensor to 10 lx, use calculated value 10001.
 *   @param {Number} value measured value, in the range 1 to 65534(0xFFFE)
 *
 * - illum_get_measured_value()
 *   Get the measured illuminance value.
 *   e.g. The returned value 10001 means 10 lx.
 *   @return {Number} measured value, in the range 1 to 65534(0xFFFE)
 *
 * - request_reporting(endpoint, command, min_interval, max_interval, threshold)
 *   Notice device server to report the attribute.
 *   @param {Object} endpoint
 *   @param {String} command 'measured_illuminance', 'thermostat_temperature', 'occupancy_sensing', 'measured_temperature'
 *   @param {Number} min_interval The reporting minimum interval. (seconds)
 *   @param {Number} max_interval The reporting maximum interval. (seconds)
 *   @param {Number} threshold Minimum changed value to trigger reporting.
 *
 * - stop_reporting(endpoint, command)
 *   Notice device server to stop the attribute reporting.
 *   @param {Object} endpoint
 *   @param {String} command 'measured_illuminance', 'thermostat_temperature', 'occupancy_sensing', 'measured_temperature'
 *
 * - reset_to_factory_default(endpoint)
 *   Resets all attribute values to factory default.
 *   @param {Object} endpoint
 */
module.exports.ZigbeeDevices = {
  /**
   * On/Off Light
   *
   * var devObject = new ZigbeeDevices.ONOFF_LIGHT(Zigbee, endpointID)
   *
   * Avaiable APIs
   * - groups_get_local_name_support()
   * - groups_set_local_name_support()
   * - onoff_get_value()
   * - ezmode_commissioning_target_start()
   * - ezmode_commissioning_target_stop()
   */
  ONOFF_LIGHT: require('../build/Release/artik-sdk.node').zigbee_onoff_light,
  /**
   * On/Off Switch
   *
   * var devObject = new ZigbeeDevices.ONOFF_SWITCH(Zigbee, endpointID)
   *
   * Avaiable APIs
   * - identify_request()
   * - identify_get_remaining_time()
   * - onoff_command()
   * - ezmode_commissioning_initiator_start()
   * - ezmode_commissioning_initiator_stop()
   */
  ONOFF_SWITCH: require('../build/Release/artik-sdk.node').zigbee_onoff_switch,
  /**
   * Level Control Switch
   *
   * var devObject = new ZigbeeDevices.LEVELCONTROL_SWITCH(Zigbee, endpointID)
   *
   * Avaiable APIs
   * - identify_request()
   * - identify_get_remaining_time()
   * - onoff_command()
   * - level_control_request()
   * - ezmode_commissioning_initiator_start()
   * - ezmode_commissioning_initiator_stop()
   */
  LEVELCONTROL_SWITCH: require('../build/Release/artik-sdk.node').zigbee_levelcontrol_switch,
  /**
   * Dimmable Light
   *
   * var devObject = new ZigbeeDevices.DIMMABLE_LIGHT(Zigbee, endpointID)
   *
   * Avaiable APIs
   * - groups_get_local_name_support()
   * - groups_set_local_name_support()
   * - onoff_get_value()
   * - level_control_get_value()
   * - ezmode_commissioning_target_start()
   * - ezmode_commissioning_target_stop()
   */
  DIMMABLE_LIGHT: require('../build/Release/artik-sdk.node').zigbee_dimmable_light,
  /**
   * Light Sensor
   *
   * var devObject = new ZigbeeDevices.LIGHT_SENSOR(Zigbee, endpointID)
   *
   * Avaiable APIs
   * - identify_request()
   * - identify_get_remaining_time()
   * - illum_set_measured_value_range()
   * - illum_set_measured_value()
   * - illum_get_measured_value()
   * - ezmode_commissioning_initiator_start()
   * - ezmode_commissioning_initiator_stop()
   */
  LIGHT_SENSOR: require('../build/Release/artik-sdk.node').zigbee_light_sensor,
  /**
   * Remote Control
   *
   * var devObject = new ZigbeeDevices.REMOTE_CONTROL(Zigbee, endpointID)
   *
   * Avaiable APIs
   * - reset_to_factory_default()
   * - identify_request()
   * - identify_get_remaining_time()
   * - onoff_command()
   * - level_control_request()
   * - request_reporting()
   * - stop_reporting()
   * - ezmode_commissioning_target_start()
   * - ezmode_commissioning_target_stop()
   */
  REMOTE_CONTROL: require('../build/Release/artik-sdk.node').zigbee_remote_control
}
