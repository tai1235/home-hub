var events = require('events');
var util = require('util');
const fs = require('fs');
var security = require('../build/Release/artik-sdk.node').security;

const begin_cert = "-----BEGIN CERTIFICATE-----\n";
const end_cert = "-----END CERTIFICATE-----\n";

const error_code = {
	"0": 0,
	"-1": -1,
	"-7001": -2,
	"-7002": -3,
	"-7003": -4,
	"-7004": -5,
	"-7005": -6,
	"-7006": -7,
}

jsonReturn = function (err, reason, pkcs7_signing_time) {
	var json = {
		error: (err != 0),
		reason: reason,
		error_code: error_code[String(err)],
		signing_time: pkcs7_signing_time,
	}
	return json;
};

var Security = function() {
    this.security = new security();
}

module.exports = Security;

Security.prototype.get_certificate = function (cert_id) {
    return this.security.get_certificate(cert_id);
}

Security.prototype.get_ca_chain = function (cert_id) {
    return this.security.get_ca_chain(cert_id);
}

Security.prototype.get_key_from_cert = function (certificate) {
    return this.security.set_network_config(certificate);
}

Security.prototype.get_random_bytes = function (number) {
    return this.security.get_random_bytes(number);
}

Security.prototype.get_certificate_sn = function (cert_id) {
    return this.security.get_certificate_sn(cert_id);
}

Security.prototype.verify_signature_init = function (signature_pem, root_ca, date, func) {
    return this.security.verify_signature_init(signature_pem, root_ca, date, func);
}

Security.prototype.verify_signature_update = function (buffer, func) {
    return this.security.verify_signature_update(buffer, func);
}

Security.prototype.verify_signature_final = function (func) {
    return this.security.verify_signature_final(func);
}

Security.prototype.pkcs7_sig_verify = function(path_signature_pem,
	path_root_ca, path_signed_data, se_id, date, return_cb) {
	var _ = this;
	/* Get content of the pem signature file */
	try {
		/* Read file signature_pem */
		var signature_pem = String(fs.readFileSync(path_signature_pem));
	} catch (err) {
		 return return_cb(jsonReturn(-1, "Cannot read PKCS7 signature file", undefined));
	}
	if (se_id == "")
	{
		/* Get content of the root_ca file */
		try {
			/* Read file root_ca */
			var root_ca = String(fs.readFileSync(path_root_ca));
		} catch (err) {
			return return_cb(jsonReturn(err, "Cannot read root CA file", undefined));
		}
	} else {
		/* Get content of the root_ca file */
		try {
			/* Read file root_ca */
			var ca_pem = this.security.get_ca_chain(se_id);

			var len_start_cert = ca_pem.search(begin_cert);
			if (len_start_cert == -1)
				return return_cb(jsonReturn(err, "Malformed PEM certificate", undefined));

			var len_end_cert = ca_pem.search(end_cert);
			if (len_end_cert == -1)
				return return_cb(jsonReturn(err, "Malformed PEM certificate", undefined));

			var root_ca = ca_pem.substr(len_start_cert, len_end_cert + end_cert.length);

		} catch (err) {
			return return_cb(jsonReturn(err, "Failed to get ca chain in secure element", undefined));
		}
	}

	/* Get pkcs7_signing_time with verify_signature_init */
	this.security.verify_signature_init(signature_pem, root_ca, date,
		function(err, reason, pkcs7_signing_time) {

		if (err != 0)
			return return_cb(jsonReturn(err, reason, pkcs7_signing_time));

		var readStream = fs.createReadStream(path_signed_data);

		readStream.on('data', function (buffer) {
			_.security.verify_signature_update(buffer, function(err, reason) {
				if (err)
					return return_cb(jsonReturn(err, reason, pkcs7_signing_time));
			});
		});

		readStream.on('end', function () {
			/* Use verify_signature_final to check package signature */
			_.security.verify_signature_final(function(err, reason) {
				if (err)
					return return_cb(jsonReturn(err, reason, pkcs7_signing_time));

				return return_cb(jsonReturn(err, "Verification successful", pkcs7_signing_time));
			});
		});

		readStream.on('error', function (err) {
			return return_cb(jsonReturn(err, "readstream failed", pkcs7_signing_time));
		});
	});
};
