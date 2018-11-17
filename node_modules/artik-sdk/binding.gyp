{
  'targets': [
    {
      'target_name': 'artik-sdk',
      'include_dirs': [
                        '<!@(pkg-config libartik-sdk-base --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-base --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-bluetooth --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-bluetooth --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-connectivity --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-connectivity --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-media --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-media --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-sensor --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-sensor --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-systemio --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-systemio --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-wifi --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-wifi --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-zigbee --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-zigbee --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-lwm2m --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-lwm2m --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        '<!@(pkg-config libartik-sdk-mqtt --cflags-only-I | sed s/-I//g)',
                        '<!@(pkg-config libartik-sdk-mqtt --cflags-only-I | sed s/-I//g | awk \'{print $1"/cpp"}\' )',
                        'addon',
                        "<!(node -e \"require('nan')\")"
      ],
      'cflags_cc': [
                '-fexceptions',
		'-DCONFIG_RELEASE',
                '<!@(pkg-config --cflags-only-I glib-2.0)'
      ],
      'link_settings' : {
                'ldflags': ['-Wl,--no-as-needed'],
                'libraries': [
                        '<!@(pkg-config --libs-only-l libartik-sdk-base)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-systemio)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-connectivity)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-media)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-sensor)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-bluetooth)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-wifi)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-zigbee)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-lwm2m)',
                        '<!@(pkg-config --libs-only-l libartik-sdk-mqtt)'
                ]
      },
      'sources': [
        'addon/artik.cc',
        'addon/utils.cc',
        'addon/loop.cc',
        'addon/base/ssl_config_converter.cc',
        'addon/gpio/gpio.cc',
        'addon/serial/serial.cc',
        'addon/i2c/i2c.cc',
        'addon/pwm/pwm.cc',
        'addon/adc/adc.cc',
        'addon/http/http.cc',
        'addon/websocket/websocket.cc',
        'addon/cloud/cloud.cc',
        'addon/wifi/wifi.cc',
        'addon/media/media.cc',
        'addon/spi/spi.cc',
        'addon/time/time.cc',
        'addon/sensor/sensor.cc',
        'addon/network/network.cc',
        'addon/bluetooth/bluetooth.cc',
        'addon/bluetooth/agent.cc',
        'addon/bluetooth/avrcp.cc',
        'addon/bluetooth/a2dp.cc',
        'addon/bluetooth/ftp.cc',
        'addon/bluetooth/gatt_client.cc',
        'addon/bluetooth/gatt_server.cc',
        'addon/bluetooth/pan.cc',
        'addon/bluetooth/spp.cc',
        'addon/zigbee/zigbee.cc',
        'addon/zigbee/zigbee_util.cc',
        'addon/zigbee/zigbee_device.cc',
        'addon/lwm2m/lwm2m.cc',
        'addon/mqtt/mqtt.cc',
        'addon/security/security.cc'
      ],
    }
  ]
}

