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
var artik      = require('../src');


/* Test Specific Includes */
var use_tls 	= process.env.WEBSOCKET_ENABLE_TLS == 1 ? true : false;
var verify		= process.env.WEBSOCKET_VERIFY_CA == 1 ? true : false;
var uri 		= use_tls ? "wss://echo.websocket.org/" : "ws://echo.websocket.org/";
var test_message 	= "ping";

var echo_websocket_ca_root =
    "-----BEGIN CERTIFICATE-----\n" +
    "MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\r\n" +
    "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\r\n" +
    "EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\r\n" +
    "ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\r\n" +
    "NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\r\n" +
    "EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\r\n" +
    "AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\r\n" +
    "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\r\n" +
    "E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\r\n" +
    "/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\r\n" +
    "DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\r\n" +
    "GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\r\n" +
    "tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\r\n" +
    "AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\r\n" +
    "FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\r\n" +
    "WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\r\n" +
    "9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\r\n" +
    "gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\r\n" +
    "2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\r\n" +
    "LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\r\n" +
    "4uJEvlz36hz1\r\n" +
    "-----END CERTIFICATE-----\n";

var ssl_config = {
    ca_cert: Buffer.from(echo_websocket_ca_root), // CA root certificate of echo.websocket.org
    verify_cert: verify ? "required" : "none"
}
var conn;
/* Test Case Module */
testCase('Websockets', function() {

	this.timeout(10000);

	pre(function() {

		conn = new artik.websocket(uri, ssl_config);

	});

	testCase('#open_stream(), on(connected)', function() {

		assertions('Return callback event when the websocket is connected', function(done) {

			conn.on('connected', function(result) {
				console.log("Connect result: " + result);
				assert.equal(result, "CONNECTED");
				done();
			});

			conn.open_stream();
		});

	});


	testCase('#write_stream(), on(receive) with echo', function () {

		assertions('Return callback event when the echo is received', function(done) {

			conn.on('receive', function(message) {
				console.log("received: " + message);
				assert.isNotNull(message);
				assert.equal(message, "ping");
				done();
			});

			console.log("sending: " + test_message);
			conn.write_stream(test_message);
		});

	});

	post(function() {
	});

});
