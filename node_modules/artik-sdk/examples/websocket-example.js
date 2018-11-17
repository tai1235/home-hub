var artik = require('../src');
var opt = require('getopt'); 

var uri = "ws://echo.websocket.org/";
var test_message = 'ping';
var verify = false;

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

try{
    opt.setopt("m:tv");
} catch (e){
    switch (e.type) {
        case "unknown":
            console.log("Unknown option: -%s", e.opt);
            console.log("Usage: node websocket-example.js [-m <test message>] [-t for enabling TLS] [-v for verifying CA certificate]");
            break;
        case "required":
            console.log("Required parameter for option: -%s", e.opt);
            break;
        default:
            console.dir(e);
    }
    process.exit(0);
}

opt.getopt(function (o, p){
    switch(o){
        case 'm':
            test_message = String(p);
            break;
        case 't':
            uri = "wss://echo.websocket.org/";
            break;
        case 'v':
            verify = true;
            break;
        default:
            console.log("Usage: node websocket-example.js [-m <test message>] [-t for enabling TLS] [-v for verifying CA certificate]");
            process.exit(0);
    }
});

var ssl_config = {
    ca_cert: Buffer.from(echo_websocket_ca_root), // CA root certificate of echo.websocket.org
    verify_cert: verify ? "required" : "none"
}

var conn = new artik.websocket(uri, ssl_config);

conn.open_stream();

conn.on('connected', function(result) {
    console.log("Connect result: " + result);

    if (result == "CONNECTED"){
        console.log("Sending: " + test_message)
        conn.write_stream(test_message);
    }
    else
        process.exit(0);
});

conn.on('receive', function(message) {
    console.log("Received: " + message);
});

process.on('SIGINT', function () {
    console.log("Close stream");
    conn.close_stream();
    process.exit(0);
});

setTimeout(function () {
    console.log("Time out, close stream");
    conn.close_stream();
    process.exit(0);
}, 5500);
