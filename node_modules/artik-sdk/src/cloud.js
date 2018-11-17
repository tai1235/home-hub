var events = require('events');
var util = require('util');
var cloud = require('../build/Release/artik-sdk.node').cloud;

var Cloud = function(token) {
    events.EventEmitter.call(this);
    this.cloud = new cloud(token);
}

util.inherits(Cloud, events.EventEmitter);

module.exports = Cloud;

Cloud.prototype.send_message = function send_message(device_id, message, response_cb, ssl_config) {
    if (arguments.length == 3) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.send_message(device_id, message, ssl_config, response_cb);
};

Cloud.prototype.send_action = function send_action(device_id, action, response_cb, ssl_config) {
    if (arguments.length == 3) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.send_action(device_id, action, ssl_config, response_cb);
};

Cloud.prototype.get_current_user_profile = function get_current_user_profile(response_cb, ssl_config) {
    if (arguments.length == 1) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.get_current_user_profile(ssl_config, response_cb);
};

Cloud.prototype.get_user_devices = function get_user_devices(count, properties, offset, user_id, response_cb, ssl_config) {
    if (arguments.length == 5) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.get_user_devices(count, properties, offset, user_id, ssl_config, response_cb);
};

Cloud.prototype.get_user_device_types = function get_user_device_types(count, shared, offset, user_id, response_cb, ssl_config) {
    if (arguments.length == 5) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.get_user_device_types(count, shared, offset, user_id, ssl_config, response_cb);
};

Cloud.prototype.get_user_application_properties = function get_user_application_properties(user_id, app_id, response_cb, ssl_config) {
    if (arguments.length == 3) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.get_user_application_properties(user_id, app_id, ssl_config, response_cb);
};

Cloud.prototype.add_device = function add_device(user_id, device_type_id, name, response_cb, ssl_config) {
    if (arguments.length == 4) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.add_device(user_id, device_type_id, name, ssl_config, response_cb);
};

Cloud.prototype.get_device = function get_device(device_id, properties, response_cb, ssl_config) {
    if (arguments.length == 3) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.get_device(device_id, properties, ssl_config, response_cb);
};

Cloud.prototype.get_device_token = function get_device_token(device_id, response_cb, ssl_config) {
    if (arguments.length == 2) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.get_device_token(device_id, ssl_config, response_cb);
};

Cloud.prototype.update_device_token = function update_device_token(device_id, response_cb, ssl_config) {
    if (arguments.length == 2) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.update_device_token(device_id, ssl_config, response_cb);
};

Cloud.prototype.delete_device_token = function delete_device_token(device_id, response_cb, ssl_config) {
    if (arguments.length == 2) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.delete_device_token(device_id, ssl_config, response_cb);
};

Cloud.prototype.delete_device = function delete_device(device_id, response_cb, ssl_config) {
    if (arguments.length == 2) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.delete_device(device_id, ssl_config, response_cb);
};

Cloud.prototype.get_device_properties = function get_device_properties(device_id, timestamp, response_cb, ssl_config) {
    if (arguments.length == 3) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.get_device_properties(device_id, timestamp, ssl_config, response_cb);
}

Cloud.prototype.set_device_server_properties = function set_device_server_properties(device_id, data, response_cb, ssl_config) {
    if (arguments.length == 3) {
        if (typeof(response_cb) == "object") {
            ssl_config = response_cb;
            response_cb = undefined;
        }
    }
    return this.cloud.set_device_server_properties(device_id, data, ssl_config, response_cb);
}

Cloud.prototype.websocket_open_stream = function websocket_open_stream(access_token, device_id, ssl_config) {
    var _ = this;
    return this.cloud.websocket_open_stream(access_token, device_id, ssl_config,
            function(message) {
                _.emit('receive', message);
            },
            function(status) {
                _.emit('connected', status);
            });
};

Cloud.prototype.websocket_send_message = function websocket_send_message(message) {
    return this.cloud.websocket_send_message(message);
};

Cloud.prototype.websocket_close_stream = function websocket_close_stream() {
    return this.cloud.websocket_close_stream();
};

Cloud.prototype.sdr_start_registration = function sdr_start_registration(cert_id, device_type_id, vendor_id, response_cb) {
    return this.cloud.sdr_start_registration(cert_id, device_type_id, vendor_id, response_cb);
};

Cloud.prototype.sdr_registration_status = function sdr_registration_status(cert_id, registration_id, response_cb) {
    return this.cloud.sdr_registration_status(cert_id, registration_id, response_cb);
}

Cloud.prototype.sdr_complete_registration = function sdr_complete_registration(cert_id, registration_id, nonce, response_cb) {
    return this.cloud.sdr_complete_registration(cert_id, registration_id, nonce, response_cb);
};
