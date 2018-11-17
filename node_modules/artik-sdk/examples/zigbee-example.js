var Zigbee = require('../src/zigbee').Zigbee
var Light = require('../src/zigbee').ZigbeeDevices.ONOFF_LIGHT
var Switch = require('../src/zigbee').ZigbeeDevices.ONOFF_SWITCH
var LevelSwitch = require('../src/zigbee').ZigbeeDevices.LEVELCONTROL_SWITCH
var DimmableLight = require('../src/zigbee').ZigbeeDevices.DIMMABLE_LIGHT
var LightSensor = require('../src/zigbee').ZigbeeDevices.LIGHT_SENSOR
var RemoteControl = require('../src/zigbee').ZigbeeDevices.REMOTE_CONTROL

var colors = require('colors')

var devices = []
var remoteEndpoints = []

var cli = [
  ' ZigBee Example program'.grey,
  function () {
    console.log(' - Local devices:', devices)
    console.log(' - Remote endpoints:', remoteEndpoints)
  },
  '-'
]

var znode = new Zigbee()

znode.on('receive_command', function (obj) { console.log(obj) })
znode.on('reporting_configure', function (obj) { console.log(obj) })
znode.on('report_attribute', function (obj) { console.log(obj) })
znode.on('identify_feedback_start', function (obj) { console.log(obj) })
znode.on('identify_feedback_stop', function (obj) { console.log(obj) })
znode.on('network_find', function (obj) { console.log(obj) })
znode.on('device_left', function (obj) { console.log(obj) })
znode.on('broadcast_identify_query_response', function (obj) { console.log(obj) })
znode.on('groups_info', function (obj) { console.log(obj) })
znode.on('commissioning_status', function (obj) { console.log(obj) })
znode.on('commissioning_target_info', function (obj) { console.log(obj) })
znode.on('commissioning_bound_info', function (obj) { console.log(obj) })
znode.on('ieee_addr', function (obj) { console.log(obj) })
znode.on('simple_desc', function (obj) { console.log(obj) })
znode.on('match_desc', function (obj) { console.log(obj) })
znode.on('basic_reset_to_factory', function (obj) { console.log(obj) })
znode.on('level_control', function (obj) { console.log(obj) })

znode.on('network_notification', function (event) {
  console.log('network notification'.blue, event)
  if (event.status === 'find_form') {
    console.log('permitjoin(60 secs)')
    znode.network_permitjoin(60)
  }
})

znode.on('attribute_change', function (event) {
  devices.forEach(function (dev) {
    if (dev.endpoint_id !== event.endpoint_id) {
      return
    }
    console.log('matched device is '.blue, dev)
    if (event.attr === 'onoff_status') {
      console.log('onoff:', dev.onoff_get_value())
    } else if (event.attr === 'levelcontrol_level') {
      console.log('level:', dev.level_control_get_value())
    }
  })
})

znode.on('device_discover', function (event) {
  if (event.status === 'start') {
    console.log('device discover status:', event.status)
    remoteEndpoints = []
  } else if (event.status === 'found') {
    console.log('device found:', event.device)
    event.device.endpoints.forEach(function (ep) {
      console.log('add endpoint:', ep)
      remoteEndpoints.push(ep)
    })
  } else {
    console.log('device discover status:', event.status)
  }
})

cli.push(' Basic initialization')
cli.push({
  cmd: 'add_device',
  args: '{type}',
  help: ['Add device', '- {type}: onoff_light/onoff_switch/...'],
  completions: ['onoff_light', 'onoff_switch', 'levelcontrol_switch', 'dimmable_light', 'light_sensor', 'remote_control'],
  func: function (args) {
    args.forEach(function (name) {
      if (name === 'onoff_light') {
        devices.push(new Light(znode, 19))
      } else if (name === 'onoff_switch') {
        devices.push(new Switch(znode, 1))
      } else if (name === 'levelcontrol_switch') {
        devices.push(new LevelSwitch(znode, 2))
      } else if (name === 'dimmable_light') {
        devices.push(new DimmableLight(znode, 20))
      } else if (name === 'light_sensor') {
        devices.push(new LightSensor(znode, 25))
      } else if (name === 'remote_control') {
        devices.push(new RemoteControl(znode, 34))
      }
    })
    console.log(devices)
  }
})

cli.push({
  cmd: 'init',
  help: 'initialize()',
  func: function () {
    znode.initialize()
  }
})

cli.push({
  cmd: 'start',
  help: 'network_start()',
  func: function () {
    try {
      var state = znode.network_start()
      console.log('network state ='.blue, state.blue)
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push('-')
cli.push(' Network control - manual')

cli.push({
  cmd: 'form',
  help: 'network_form()',
  func: function () {
    znode.network_form()
  }
})

cli.push({
  cmd: 'permitjoin',
  args: '{N:seconds}',
  help: 'network_permitjoin(60)',
  func: function (args) {
    try {
      znode.network_permitjoin(args[1] ? +args[1] : 60)
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'join',
  help: 'network_join()',
  func: function () {
    znode.network_join()
  }
})

cli.push({
  cmd: 'leave',
  help: 'network_leave()',
  func: function () {
    try {
      znode.network_leave()
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'reset_local',
  help: 'reset_local()',
  func: function () {
    try {
      znode.reset_local()
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push('-')
cli.push(' Network control - ezmode')

cli.push({
  cmd: 'ez_target_start',
  help: 'ezmode_commissioning_target_start()',
  func: function () {
    try {
      devices.forEach(function (dev) {
        dev.ezmode_commissioning_target_start()
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'ez_target_stop',
  help: 'ezmode_commissioning_target_stop()',
  func: function () {
    try {
      devices.forEach(function (dev) {
        dev.ezmode_commissioning_target_stop()
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'ez_initiator_start',
  help: 'ezmode_commissioning_initiator_start()',
  func: function () {
    try {
      devices.forEach(function (dev) {
        dev.ezmode_commissioning_initiator_start()
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'ez_initiator_stop',
  help: 'ezmode_commissioning_initiator_stop()',
  func: function () {
    try {
      devices.forEach(function (dev) {
        dev.ezmode_commissioning_initiator_stop()
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push('-')
cli.push(' Utility functions')

cli.push({
  cmd: 'get_nstatus',
  help: 'network_request_my_network_status()',
  func: function () {
    try {
      var state = znode.network_request_my_network_status()
      console.log('network_status ='.blue, state.blue)
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_dntype',
  help: 'device_request_my_node_type()',
  func: function () {
    try {
      var ntype = znode.device_request_my_node_type()
      console.log('device_node_type ='.blue, ntype.blue)
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'find_device',
  args: '{N:cluster_id}',
  help: 'device_find_by_cluster()',
  func: function (args) {
    var dev = znode.device_find_by_cluster(+args[1])
    console.log('device ='.blue, dev)
  }
})

cli.push({
  cmd: 'discover',
  help: 'device_discover()',
  func: function () {
    try {
      znode.device_discover()
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'set_discover_time',
  args: '{N:minute}',
  help: 'set_discover_cycle_time(minute)',
  func: function () {
    try {
      var minute = 1
      znode.set_discover_cycle_time(minute)
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_discov_devices',
  help: 'get_discovered_device_info()',
  func: function () {
    try {
      var dev = znode.get_discovered_device_list()
      console.log('device ='.blue, colors.blue(JSON.stringify(dev, null, 2)))
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_local_devices',
  help: 'get_local_device_list()',
  func: function () {
    var dev = znode.get_local_device_list()
    console.log('devices ='.blue, colors.blue(JSON.stringify(dev, null, 2)))
  }
})

cli.push('-')
cli.push(' Cluster command')

cli.push({
  cmd: 'set_onoff',
  args: '{on/off/toggle}',
  help: 'Send ON / OFF Command',
  completions: ['on', 'off', 'toggle'],
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        remoteEndpoints.forEach(function (ep) {
          console.log('try endpoint -', ep.endpoint_id)
          dev.onoff_command(ep, args[1])
        })
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_onoff',
  help: 'onoff_get_value()',
  func: function () {
    try {
      devices.forEach(function (dev) {
        var value = dev.onoff_get_value()
        console.log('onoff value ='.blue, colors.blue(value))
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'set_level',
  args: '{type} {N:level} {t}',
  help: [ 'level_control_request()', '- {type}: moveto/moveup/movedown/...', '- {t}: transition time (1/10 secs)' ],
  completions: ['moveup', 'movedown', 'moveto', 'stepup', 'stepdown', 'stop'],
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        remoteEndpoints.forEach(function (ep) {
          console.log('try endpoint -', ep.endpoint_id)
          dev.level_control_request(ep, {
            type: args[1],
            value: +args[2],
            auto_onoff: true,
            transition_time: (args[3] === undefined) ? 10 : +args[3] /* default 1 sec */
          })
        })
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_level',
  help: 'level_control_get_value()',
  func: function () {
    try {
      devices.forEach(function (dev) {
        var value = dev.level_control_get_value()
        console.log('level value ='.blue, colors.blue(value))
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'set_illum_range',
  args: '{N:min} {N:max}',
  help: 'illum_set_measured_value_range()',
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        dev.illum_set_measured_value_range(+args[1], +args[2])
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'set_illum_value',
  args: '{N:value}',
  help: 'illum_set_measured_value()',
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        dev.illum_set_measured_value(+args[1])
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_illum_value',
  help: 'illum_get_measured_value()',
  func: function () {
    try {
      devices.forEach(function (dev) {
        var value = dev.illum_get_measured_value()
        console.log('illum value ='.blue, colors.blue(value))
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'req_reporting',
  args: '{t} {min} {max} {th}',
  help: ['request_reporting()', '- {type}: measured_illuminance/...', '- {min}/{max}: interval', '- {th}: change threshold'],
  completions: ['thermostat_temperature', 'occupancy_sensing', 'measured_illuminance', 'measured_temperature'],
  func: function (args) {
    if (args.length !== 5) {
      console.log('{type} {min_interval} {max_interval} {threshold}'.red)
      return
    }

    try {
      devices.forEach(function (dev) {
        remoteEndpoints.forEach(function (ep) {
          console.log('try endpoint -', ep.endpoint_id)
          dev.request_reporting(ep, args[1], +args[2], +args[3], +args[4])
        })
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'stop_reporting',
  args: '{type}',
  help: ['stop_reporting()', '- {type}: measured_illuminance/...'],
  completions: ['thermostat_temperature', 'occupancy_sensing', 'measured_illuminance', 'measured_temperature'],
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        remoteEndpoints.forEach(function (ep) {
          console.log('try endpoint -', ep.endpoint_id)
          dev.stop_reporting(ep, args[1])
        })
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'reset_to_factory',
  help: 'reset_to_factory_default',
  func: function () {
    try {
      devices.forEach(function (dev) {
        remoteEndpoints.forEach(function (ep) {
          console.log('try endpoint -', ep.endpoint_id)
          dev.reset_to_factory_default(ep)
        })
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'req_identify',
  help: 'identify_request(duration)',
  args: '{N:seconds}',
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        remoteEndpoints.forEach(function (ep) {
          console.log('try endpoint -', ep.endpoint_id)
          dev.identify_request(ep, +args[1])
        })
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_ir_time',
  help: 'identify_get_remaining_time()',
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        remoteEndpoints.forEach(function (ep) {
          console.log('try endpoint -', ep.endpoint_id)
          var remain = dev.identify_get_remaining_time(ep)
          console.log('remaining time ='.blue, colors.blue(remain))
        })
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'get_gname_support',
  help: 'groups_get_local_name_support()',
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        console.log('try endpoint -', dev.endpoint_id)
        var isSupport = dev.groups_get_local_name_support()
        console.log('supported =', isSupport)
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push({
  cmd: 'set_gname_support',
  help: 'groups_set_local_name_support()',
  args: '{true/false}',
  completions: ['true', 'false'],
  func: function (args) {
    try {
      devices.forEach(function (dev) {
        console.log('try set to', args[1])
        dev.groups_set_local_name_support((args[1] === 'true'))
      })
    } catch (e) {
      console.log(colors.red(e))
    }
  }
})

cli.push('-')

cli.push({
  cmd: 'help',
  help: 'Show help',
  func: help
})

cli.push({
  cmd: 'quit',
  help: 'Quit (same as exit)',
  func: function () {
    process.exit(0)
  }
})

cli.push({
  cmd: 'exit',
  help: 'Exit (same as quit)',
  func: function () {
    process.exit(0)
  }
})

cli.push('-')

function help () {
  cli.forEach(function (item) {
    if (item === '-') {
      console.log('-'.repeat(78))
    } else if (typeof item === 'string') {
      console.log(item)
    } else if (typeof item === 'function') {
      item()
    } else {
      var left = (' '.repeat(20) + item.cmd).slice(-20).blue + ' ' +
        ((item.args ? item.args : '') + ' '.repeat(20)).slice(0, 20).green + ' '

      if (typeof item.help !== 'string') {
        console.log(left, item.help[0])

        left = ' '.repeat(20) + ' ' + ' '.repeat(20) + ' '
        item.help.forEach(function (row, index) {
          if (index === 0) {
            return
          }
          console.log(left, row)
        })
      } else {
        console.log(left, item.help)
      }
    }
  })
}

var rl = require('readline').createInterface({
  input: process.stdin,
  output: process.stdout,
  completer: function (line) {
    var completions = []
    var cur = line.trim().split(' ')

    cli.forEach(function (item) {
      if (cur[0] === item.cmd && item.completions) {
        item.completions.forEach(function (param) {
          completions.push(item.cmd + ' ' + param)
        })
      }
    })

    if (completions.length === 0) {
      /* main completions */
      cli.forEach(function (item) {
        if (item.cmd) {
          completions.push(item.cmd)
        }
      })
    }

    var hits = completions.filter(function (c) {
      if (c.indexOf(line) === 0) {
        return c
      }
    })

    return [hits && hits.length ? hits : [], line]
  }
})

rl.on('line', function (line) {
  var args = line.trim().split(' ')

  cli.forEach(function (item) {
    if (item.cmd === undefined) {
      return
    }

    if (args[0] === item.cmd) {
      if (item.args && args.length === 1) {
        console.log('Please input with additional parameters.'.red, item.args.green)
        return
      }

      item.func(args)
    }
  })

  rl.prompt()
})

rl.on('close', function () {
  process.exit(0)
})

help()
rl.setPrompt('ZigBee > ')
rl.prompt()
