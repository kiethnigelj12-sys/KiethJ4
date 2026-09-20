#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DHT.h>
#include <time.h>


// =========================================================
// FIREBASE CONFIG - NIGEL
// =========================================================

#define API_KEY \
"AIzaSyAlvefyVlWc1CYCJupACS0OBI13u62E56Q"

#define DATABASE_URL \
"https://esp32-monitoring-de209-default-rtdb.europe-west1.firebasedatabase.app"


#define USER_EMAIL \
"kiethnigelj12@gmail.com"

#define USER_PASSWORD \
"Iloveyoumore2003"


// =========================================================
// DHT11
// =========================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(
    DHTPIN,
    DHTTYPE
);


// =========================================================
// WIFI MANAGER FILES
// =========================================================

const char *ssidFile =
    "/ssid.txt";

const char *passFile =
    "/pass.txt";

const char *ipFile =
    "/ip.txt";

const char *gatewayFile =
    "/gateway.txt";


// =========================================================
// WIFI MANAGER AP
// =========================================================

const char *AP_SSID =
    "ESP-WIFI-MANAGER";


// =========================================================
// WEB SERVER
// =========================================================

AsyncWebServer server(80);


// =========================================================
// FIREBASE
// =========================================================

WiFiClientSecure ssl_client;
AsyncClientClass aClient(ssl_client);


UserAuth user_auth(
    API_KEY,
    USER_EMAIL,
    USER_PASSWORD
);

FirebaseApp app;

RealtimeDatabase Database;


// =========================================================
// SENSOR INTERVAL
// =========================================================
// Automatic reading only.
// WALANG interval setting sa website.
// 5000 = 5 seconds.
// =========================================================

const unsigned long SENSOR_INTERVAL =
    5000;

unsigned long lastSensorReading =
    0;


// =========================================================
// WIFI MANAGER MODE
// =========================================================

bool wifiManagerMode =
    false;


// =========================================================
// READ FILE
// =========================================================

String readFile(
    const char *path
) {

    if (
        !LittleFS.exists(path)
    ) {

        return "";

    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file) {

        return "";

    }


    String value =
        file.readString();

    file.close();

    value.trim();

    return value;
}


// =========================================================
// WRITE FILE
// =========================================================

bool writeFile(
    const char *path,
    const String &value
) {

    File file =
        LittleFS.open(
            path,
            "w"
        );


    if (!file) {

        return false;

    }


    file.print(
        value
    );

    file.close();

    return true;
}


// =========================================================
// START WIFI MANAGER
// =========================================================

void startWiFiManager() {

    wifiManagerMode =
        true;


    WiFi.mode(
        WIFI_AP
    );


    WiFi.softAP(
        AP_SSID
    );


    IPAddress apIP =
        WiFi.softAPIP();


    Serial.println();

    Serial.println(
        "========================================"
    );

    Serial.println(
        "         WIFI MANAGER MODE"
    );

    Serial.println(
        "========================================"
    );

    Serial.print(
        "AP SSID: "
    );

    Serial.println(
        AP_SSID
    );

    Serial.print(
        "Manager URL: http://"
    );

    Serial.println(
        apIP
    );


    // =====================================================
    // WIFI MANAGER PAGE
    // =====================================================

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request) {

            if (
                LittleFS.exists(
                    "/wifimanager.html"
                )
            ) {

                request->send(
                    LittleFS,
                    "/wifimanager.html",
                    "text/html"
                );

            } else {

                request->send(
                    200,
                    "text/plain",
                    "wifimanager.html not found in LittleFS."
                );

            }

        }
    );


    // =====================================================
    // SAVE WIFI SETTINGS
    // =====================================================

    server.on(
        "/",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {

            String ssid =
                "";

            String password =
                "";

            String ip =
                "";

            String gateway =
                "";


            // -----------------------------
            // SSID
            // -----------------------------

            if (
                request->hasParam(
                    "ssid",
                    true
                )
            ) {

                ssid =
                    request
                        ->getParam(
                            "ssid",
                            true
                        )
                        ->value();

            }


            // -----------------------------
            // PASSWORD
            // -----------------------------

            if (
                request->hasParam(
                    "pass",
                    true
                )
            ) {

                password =
                    request
                        ->getParam(
                            "pass",
                            true
                        )
                        ->value();

            }


            // -----------------------------
            // IP
            // -----------------------------

            if (
                request->hasParam(
                    "ip",
                    true
                )
            ) {

                ip =
                    request
                        ->getParam(
                            "ip",
                            true
                        )
                        ->value();

            }


            // -----------------------------
            // GATEWAY
            // -----------------------------

            if (
                request->hasParam(
                    "gateway",
                    true
                )
            ) {

                gateway =
                    request
                        ->getParam(
                            "gateway",
                            true
                        )
                        ->value();

            }


            ssid.trim();
            password.trim();
            ip.trim();
            gateway.trim();


            // -----------------------------
            // CHECK SSID
            // -----------------------------

            if (
                ssid.length() == 0
            ) {

                request->send(
                    400,
                    "text/plain",
                    "SSID is required."
                );

                return;

            }


            // -----------------------------
            // SAVE
            // -----------------------------

            bool savedSSID =
                writeFile(
                    ssidFile,
                    ssid
                );

            bool savedPassword =
                writeFile(
                    passFile,
                    password
                );

            bool savedIP =
                writeFile(
                    ipFile,
                    ip
                );

            bool savedGateway =
                writeFile(
                    gatewayFile,
                    gateway
                );


            if (
                !savedSSID ||
                !savedPassword ||
                !savedIP ||
                !savedGateway
            ) {

                request->send(
                    500,
                    "text/plain",
                    "Failed to save WiFi settings."
                );

                return;

            }


            // -----------------------------
            // RESPONSE
            // -----------------------------

            request->send(
                200,
                "text/html",

                "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>"
                "body{font-family:Arial;background:#fce7f3;text-align:center;padding:40px;}"
                ".box{background:white;padding:30px;border-radius:18px;max-width:450px;margin:auto;"
                "box-shadow:0 10px 30px rgba(190,24,93,.20);}"
                "h2{color:#be185d;}"
                "</style>"
                "</head>"
                "<body>"
                "<div class='box'>"
                "<h2>WiFi Settings Saved!</h2>"
                "<p>The ESP32 will restart.</p>"
                "<p>It will automatically connect to the saved WiFi.</p>"
                "</div>"
                "</body>"
                "</html>"
            );


            delay(
                1500
            );


            ESP.restart();

        }
    );


    server.begin();

}


// =========================================================
// CONNECT TO SAVED WIFI
// =========================================================

bool connectSavedWiFi() {

    String ssid =
        readFile(
            ssidFile
        );

    String password =
        readFile(
            passFile
        );

    String ip =
        readFile(
            ipFile
        );

    String gateway =
        readFile(
            gatewayFile
        );


    // =====================================================
    // NO SAVED WIFI
    // =====================================================

    if (
        ssid.length() == 0
    ) {

        Serial.println();

        Serial.println(
            "NO CONNECTED WIFI"
        );

        return false;

    }


    Serial.println();

    Serial.println(
        "========================================"
    );

    Serial.println(
        "       CONNECTING TO SAVED WIFI"
    );

    Serial.println(
        "========================================"
    );

    Serial.print(
        "SSID: "
    );

    Serial.println(
        ssid
    );


    // =====================================================
    // STATIC IP
    // =====================================================

    if (
        ip.length() > 0 &&
        gateway.length() > 0
    ) {

        IPAddress localIP;

        IPAddress gatewayIP;

        IPAddress subnet(
            255,
            255,
            255,
            0
        );


        if (
            localIP.fromString(ip) &&
            gatewayIP.fromString(gateway)
        ) {

            WiFi.config(
                localIP,
                gatewayIP,
                subnet
            );


            Serial.print(
                "Static IP: "
            );

            Serial.println(
                localIP
            );

        }

    } else {

        Serial.println(
            "Using DHCP."
        );

    }


    // =====================================================
    // CONNECT
    // =====================================================

    WiFi.mode(
        WIFI_STA
    );


    WiFi.begin(
        ssid.c_str(),
        password.c_str()
    );


    unsigned long startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 15000
    ) {

        delay(
            500
        );

        Serial.print(
            "."
        );

    }


    Serial.println();


    // =====================================================
    // FAILED
    // =====================================================

    if (
        WiFi.status() != WL_CONNECTED
    ) {

        Serial.println(
            "WIFI CONNECTION FAILED"
        );

        return false;

    }


    // =====================================================
    // SUCCESS
    // =====================================================

    Serial.println(
        "WIFI CONNECTED!"
    );


    Serial.print(
        "ESP32 IP: "
    );

    Serial.println(
        WiFi.localIP()
    );


    return true;
}


// =========================================================
// FIREBASE CALLBACK
// =========================================================

void processFirebase(
    AsyncResult &aResult
) {

    if (
        aResult.isEvent()
    ) {

        Firebase.printf(
            "Firebase Event - task: %s, msg: %s, code: %d\n",

            aResult.uid().c_str(),

            aResult.eventLog()
                .message()
                .c_str(),

            aResult.eventLog()
                .code()
        );

    }


    if (
        aResult.isDebug()
    ) {

        Firebase.printf(
            "Firebase Debug - task: %s, msg: %s\n",

            aResult.uid().c_str(),

            aResult.debug().c_str()
        );

    }


    if (
        aResult.isError()
    ) {

        Firebase.printf(
            "Firebase Error - task: %s, msg: %s, code: %d\n",

            aResult.uid().c_str(),

            aResult.error()
                .message()
                .c_str(),

            aResult.error()
                .code()
        );

    }


    if (
        aResult.available()
    ) {

        Firebase.printf(
            "Firebase Payload - task: %s, payload: %s\n",

            aResult.uid().c_str(),

            aResult.c_str()
        );

    }

}


// =========================================================
// FIREBASE SETUP
// =========================================================

void setupFirebase() {

    ssl_client.setInsecure();


    Serial.println();

    Serial.println(
        "Starting Firebase..."
    );


    initializeApp(
        aClient,
        app,
        getAuth(
            user_auth
        ),
        processFirebase,
        "authTask"
    );


    app.getApp<
        RealtimeDatabase
    >(
        Database
    );


    Database.url(
        DATABASE_URL
    );


    Serial.println(
        "Firebase initialized."
    );

}


// =========================================================
// TIME SETUP
// =========================================================

void setupTime() {

    configTime(
        28800,
        0,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com"
    );


    Serial.println(
        "Synchronizing time..."
    );


    struct tm timeinfo;


    for (
        int i = 0;
        i < 20;
        i++
    ) {

        if (
            getLocalTime(
                &timeinfo
            )
        ) {

            Serial.println();

            Serial.println(
                "Time synchronized."
            );

            return;

        }


        delay(
            500
        );

        Serial.print(
            "."
        );

    }


    Serial.println();

}


// =========================================================
// DATE
// =========================================================

String getDateString() {

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    ) {

        return "0000-00-00";

    }


    char buffer[11];


    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &timeinfo
    );


    return String(
        buffer
    );

}


// =========================================================
// TIME
// =========================================================

String getTimeString() {

    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    ) {

        return "00-00-00";

    }


    char buffer[9];


    strftime(
        buffer,
        sizeof(buffer),
        "%H-%M-%S",
        &timeinfo
    );


    return String(
        buffer
    );

}


// =========================================================
// SEND DHT11 DATA
// =========================================================

void sendSensorData() {

    // =====================================================
    // WIFI CHECK
    // =====================================================

    if (
        WiFi.status() != WL_CONNECTED
    ) {

        Serial.println(
            "WiFi disconnected. Sensor data not sent."
        );

        return;

    }


    // =====================================================
    // FIREBASE CHECK
    // =====================================================

    if (
        !app.ready()
    ) {

        Serial.println(
            "Firebase not ready yet."
        );

        return;

    }


    // =====================================================
    // DHT11 READING
    // =====================================================

    float humidity =
        dht.readHumidity();


    float temperature =
        dht.readTemperature();


    // =====================================================
    // SENSOR ERROR
    // =====================================================

    if (
        isnan(humidity) ||
        isnan(temperature)
    ) {

        Serial.println();

        Serial.println(
            "========================================"
        );

        Serial.println(
            "        DHT11 READ ERROR"
        );

        Serial.println(
            "========================================"
        );

        return;

    }


    // =====================================================
    // DATE AND TIME
    // =====================================================

    String date =
        getDateString();

    String time =
        getTimeString();


    // =====================================================
    // FIREBASE PATH
    // =====================================================

    String basePath =
        "/ESP32_Data/" +
        date +
        "/" +
        time;


    // =====================================================
    // SERIAL MONITOR
    // =====================================================

    Serial.println();

    Serial.println(
        "========================================"
    );

    Serial.println(
        "          DHT11 SENSOR READING"
    );

    Serial.println(
        "========================================"
    );


    Serial.print(
        "Firebase Path: "
    );

    Serial.println(
        basePath
    );


    Serial.print(
        "Temperature: "
    );

    Serial.print(
        temperature,
        1
    );

    Serial.println(
        " °C"
    );


    Serial.print(
        "Humidity: "
    );

    Serial.print(
        humidity,
        1
    );

    Serial.println(
        " %"
    );


    // =====================================================
    // WRITE TEMPERATURE
    // =====================================================

    Database.set<float>(
        aClient,

        basePath +
        "/temperature",

        temperature,

        processFirebase,

        "temperatureTask"
    );


    // =====================================================
    // WRITE HUMIDITY
    // =====================================================

    Database.set<float>(
        aClient,

        basePath +
        "/humidity",

        humidity,

        processFirebase,

        "humidityTask"
    );


    Serial.println();

    Serial.println(
        "Temperature write task sent."
    );

    Serial.println(
        "Humidity write task sent."
    );

}


// =========================================================
// MAIN WEB SERVER
// =========================================================

void startMainWebServer() {

    wifiManagerMode =
        false;


    // =====================================================
    // INDEX
    // =====================================================

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request) {

            if (
                LittleFS.exists(
                    "/index.html"
                )
            ) {

                request->send(
                    LittleFS,
                    "/index.html",
                    "text/html"
                );

            } else {

                request->send(
                    404,
                    "text/plain",
                    "index.html not found."
                );

            }

        }
    );


    // =====================================================
    // STATIC FILES
    // =====================================================

    server.serveStatic(
        "/",
        LittleFS,
        "/"
    );


    server.begin();


    Serial.println();

    Serial.println(
        "========================================"
    );

    Serial.println(
        "          MAIN WEB SERVER"
    );

    Serial.println(
        "========================================"
    );


    Serial.print(
        "Open: http://"
    );

    Serial.println(
        WiFi.localIP()
    );

}


// =========================================================
// SETUP
// =========================================================

void setup() {

    Serial.begin(
        115200
    );


    delay(
        1000
    );


    Serial.println();

    Serial.println(
        "========================================"
    );

    Serial.println(
        "         NIGEL ACTIVITY 4"
    );

    Serial.println(
        "       DHT11 MONITORING SYSTEM"
    );

    Serial.println(
        "========================================"
    );


    // =====================================================
    // LITTLEFS
    // =====================================================

    if (
        !LittleFS.begin(true)
    ) {

        Serial.println(
            "LittleFS mount failed!"
        );

        return;

    }


    Serial.println(
        "LittleFS mounted."
    );


    // =====================================================
    // DHT11
    // =====================================================

    dht.begin();


    Serial.println(
        "DHT11 initialized."
    );


    // =====================================================
    // WIFI
    // =====================================================

    if (
        !connectSavedWiFi()
    ) {

        startWiFiManager();

        return;

    }


    // =====================================================
    // NTP
    // =====================================================

    setupTime();


    // =====================================================
    // FIREBASE
    // =====================================================

    setupFirebase();


    // =====================================================
    // MAIN WEBSITE
    // =====================================================

    startMainWebServer();


    // =====================================================
    // FIRST READING
    // =====================================================

    lastSensorReading =
        millis() -
        SENSOR_INTERVAL;

}


// =========================================================
// LOOP
// =========================================================

void loop() {

    // =====================================================
    // WIFI MANAGER
    // =====================================================

    if (
        wifiManagerMode
    ) {

        return;

    }


    // =====================================================
    // FIREBASE
    // =====================================================

    app.loop();


    // =====================================================
    // AUTOMATIC SENSOR READING
    // =====================================================

    if (
        millis() -
        lastSensorReading >=
        SENSOR_INTERVAL
    ) {

        lastSensorReading =
            millis();


        sendSensorData();

    }

}