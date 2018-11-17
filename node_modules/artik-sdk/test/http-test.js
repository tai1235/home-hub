/* Global Includes */
var testCase   = require('mocha').describe;
var pre        = require('mocha').before;
var preEach    = require('mocha').beforeEach;
var post       = require('mocha').after;
var postEach   = require('mocha').afterEach;
var assertions = require('mocha').it;
var assert     = require('chai').assert;
var validator  = require('validator');
var exec       = require('child_process').execSync;
var artik_http = require("../src/http");
var md5        = require('md5');
var fs         = require('fs');

/* Test Specific Includes */
var http = new artik_http();
var headers = [
	"user-agent", "ARTIK browser",
	"Accept-Language", "en-US,en;q=0.8"
];
var body = "name=samsung&project=artik";

var IMG_MD5 = "a27095e7727c70909c910cefe16d30de";

var allow_disable_wifi = process.env.ALLOW_DISABLE_WIFI;

var httpbin_ca_root =
	"-----BEGIN CERTIFICATE-----\n" +
	"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\r\n" +
	"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\r\n" +
	"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\r\n" +
	"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\r\n" +
	"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n" +
	"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\r\n" +
	"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\r\n" +
	"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\r\n" +
	"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\r\n" +
	"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\r\n" +
	"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\r\n" +
	"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\r\n" +
	"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\r\n" +
	"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\r\n" +
	"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\r\n" +
	"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\r\n" +
	"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\r\n" +
	"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\r\n" +
	"-----END CERTIFICATE-----\n";

var ssl_config = {
	ca_cert: Buffer.from(httpbin_ca_root), // CA root certificate of httpbin.org
	verify_cert: "required"
};

/* Test Case Module */
testCase('HTTP', function() {

	pre(function() {
	});

	testCase('#get_stream()', function() {

		assertions('GET STREAM - Test download http://httpbin.org/image/jpeg image.jpeg with md5sum', function(done) {
			this.timeout(10000);

			try {

				var ws = fs.createWriteStream("./image.jpeg");
				var stream = http.get_stream('http://httpbin.org/image/jpeg', headers, null);

				stream.pipe(ws);

				ws.on('close', function() {
					console.log("Write finished");
					fs.readFile('image.jpeg', function(err, buf) {
						console.log("MD5 of downloaded file = " + md5(buf));
						console.log("MD5 expected = " + IMG_MD5);
						assert.equal(md5(buf), IMG_MD5,"Image correctly downloaded");
					});
					done();
				});

			} catch(err) {
				console.log(err);
			}

		});

		assertions('GET STREAM - Test download image http://httpbinnull.org/image/jpeg (e.g bad URL)', function(done) {
			this.timeout(10000);

			try {

				var ws = fs.createWriteStream("./image.jpeg");
				var stream = http.get_stream('http://httpbinnull.org/image/jpeg', headers, null);

				stream.pipe(ws);

				ws.on('close', function() {
					console.log("Write finished");
					fs.readFile('image.jpeg', function(err, buf) {
						console.log("MD5 of downloaded file = " + md5(buf));
						console.log("MD5 expected = " + IMG_MD5);
						assert.notEqual(md5(buf), IMG_MD5, "Image no correctly downloaded (e.g bad URL)");
					});
				});

				stream.on('error', function() {
					done();
				});

			} catch(err) {
				console.log(err);
				done();
			}

		});

	});

	testCase('#get_stream() with SSL verify required', function() {

		assertions('GET STREAM - Test download https://httpbin.org/image/jpeg image.jpeg with md5sum', function(done) {
			this.timeout(10000);

			try {

				var ws = fs.createWriteStream("./image.jpeg");
				var stream = http.get_stream('https://httpbin.org/image/jpeg', headers, ssl_config);

				stream.pipe(ws);

				ws.on('close', function() {
					console.log("Write finished");
					fs.readFile('image.jpeg', function(err, buf) {
						console.log("MD5 of downloaded file = " + md5(buf));
						console.log("MD5 expected = " + IMG_MD5);
						assert.equal(md5(buf), IMG_MD5,"Image correctly downloaded");
					});
					done();
				});

			} catch(err) {
				console.log(err);
			}

		});

		assertions('GET STREAM - Test download image https://httpbinnull.org/image/jpeg (e.g bad URL)', function(done) {
			this.timeout(10000);

			try {

				var ws = fs.createWriteStream("./image.jpeg");
				var stream = http.get_stream('https://httpbinnull.org/image/jpeg', headers, ssl_config);

				stream.on('end', function() {
					console.log("Stream finished");
				    ws.close();
				})

				stream.pipe(ws);

				ws.on('close', function() {
					console.log("Write finished");
					fs.readFile('image.jpeg', function(err, buf) {
						console.log("MD5 of downloaded file = " + md5(buf));
						console.log("MD5 expected = " + IMG_MD5);
						assert.notEqual(md5(buf), IMG_MD5, "Image no correctly downloaded (e.g bad URL)");
					});
				});

				stream.on('error', function() {
					done();
				});

			} catch(err) {
				console.log(err);
			}

		});

	});

	testCase('#post()', function() {

		assertions('HTTP POST - Should return valid response if the URL is valid', function(done) {
			http.post("https://httpbin.org/post", headers, body, null, function(response, status) {
				console.log("POST - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});

		});

		assertions('HTTP POST - Should return HTTP 404 response if the URL is invalid', function(done) {
			http.post("https://httpbin.org/postNull", headers, body, null, function(response, status) {
				console.log("POST - status " + status + " - response: " + response);
				assert.equal(status, 404);
				done();
			});

		});

	});

	testCase('#post() with SSL verify required', function() {

		assertions('HTTP POST SSL - Should return valid response if the URL is valid', function(done) {
			http.post("https://httpbin.org/post", headers, body, ssl_config, function(response, status) {
				console.log("POST - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});

		});

		assertions('HTTP POST SSL - Should return HTTP 404 response if the URL is invalid', function(done) {
			http.post("https://httpbin.org/postNull", headers, body, ssl_config, function(response, status) {
				console.log("POST - status " + status + " - response: " + response);
				assert.equal(status, 404);
				done();
			});

		});

	});

	testCase('#put()', function() {

		assertions('HTTP PUT - Should return valid response if the URL is valid', function(done) {
			http.put("https://httpbin.org/put", headers, body, null, function(response, status) {
				console.log("PUT - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});
		});

	});

	testCase('#put() with SSL verify required', function() {

		assertions('HTTP PUT SSL - Should return valid response if the URL is valid', function(done) {
			http.put("https://httpbin.org/put", headers, body, ssl_config, function(response, status) {
				console.log("PUT - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});
		});

	});

	testCase('#del()', function() {
		assertions('HTTP Delete - Should return valid response if the URL is valid', function(done) {

			http.del("https://httpbin.org/delete", headers, null, function(response, status) {
				console.log("DELETE - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});

		});
	});

	testCase('#del() with SSL verify required', function() {
		assertions('HTTP Delete SSL - Should return valid response if the URL is valid', function(done) {

			http.del("https://httpbin.org/delete", headers, ssl_config, function(response, status) {
				console.log("DELETE - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});

		});
	});

	testCase('#get()', function() {

		assertions('HTTP Get - Should return valid response if the URL is valid', function(done) {

			http.get("https://httpbin.org/get", headers, null, function(response, status) {
				console.log("GET - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});

		});

		assertions('HTTP Get - Should return HTTP 404 response if the URL is invalid', function(done) {

			http.get("https://httpbin.org/getNull", headers, null, function(response, status) {
				console.log("GET - status " + status + " - response: " + response);
				assert.equal(status, 404);
				done();
			});

		});
	});

	testCase('#get() with SSL verify required', function() {

		assertions('HTTP Get SSL - Should return valid response if the URL is valid', function(done) {

			http.get("https://httpbin.org/get", headers, ssl_config, function(response, status) {
				console.log("GET - status " + status + " - response: " + response);
				assert.equal(status, 200);
				assert.isNotNull(response);
				done();
			});

		});

		assertions('HTTP Get SSL - Should return HTTP 404 response if the URL is invalid', function(done) {

			http.get("https://httpbin.org/getNull", headers, ssl_config, function(response, status) {
				console.log("GET - status " + status + " - response: " + response);
				assert.equal(status, 404);
				done();
			});

		});
	});

	testCase('#get() - network down', function(done) {
	        pre(function() {
			if (allow_disable_wifi == 1) {
				console.log("Disabling Wifi");
				exec("ifconfig wlan0 down");
			}

		});

		assertions('HTTP Get - Should return valid response if the network is down', function(done) {
			console.log(allow_disable_wifi);
			if (allow_disable_wifi == 0)
				this.skip();

			http.get("https://httpbin.org/get", headers, null, function(response, status) {
				console.log("GET - status " + status + " - response: " + response);
				assert.equal(response, "HTTP error");
				done();
			});

		});

		post(function() {
			if (allow_disable_wifi == 1) {
				this.timeout(15000);
				exec("ifconfig wlan0 up; sleep 1; pkill dhclient; sleep 1; dhclient wlan0");
			}
		});
	});
});
