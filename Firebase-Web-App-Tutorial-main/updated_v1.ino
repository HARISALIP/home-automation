#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"
#include <ArduinoJson.h>

// ================== USER CONFIG ==================

#define WIFI_SSID     "RASHIF"
#define WIFI_PASSWORD "123456789"

#define Web_API_KEY   "AIzaSyDcV-SV8W0klraHxMTg3O3vCh5eSjxXG2c"
#define DATABASE_URL  "https://home-automation-24fcc-default-rtdb.asia-southeast1.firebasedatabase.app"

#define USER_EMAIL    "harisalisg@gmail.com"
#define USER_PASS     "123456"

// =================================================

// Firebase Authentication
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

SSL_CLIENT ssl_client, stream_ssl_client;

FirebaseApp app;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client);
AsyncClient streamClient(stream_ssl_client);

RealtimeDatabase Database;

// GPIO Pins
const int GPIO1 = 12;
const int GPIO2 = 13;
const int GPIO3 = 14;

// Function Declaration
void processData(AsyncResult &aResult);

// ================= WIFI =================

void initWiFi()
{
    Serial.println();
    Serial.println("================================");
    Serial.println("Connecting To WiFi");
    Serial.println("================================");

    Serial.print("SSID : ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int retry = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        retry++;

        if (retry > 60)
        {
            Serial.println("\nWiFi Connection Failed!");
            ESP.restart();
        }
    }

    Serial.println();
    Serial.println("================================");
    Serial.println("WiFi Connected Successfully");
    Serial.println("================================");

    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI : ");
    Serial.println(WiFi.RSSI());

    Serial.println("================================");
}

// ================= SETUP =================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("ESP8266 Firebase Home Automation");
    Serial.println("================================");

    pinMode(GPIO1, OUTPUT);
    pinMode(GPIO2, OUTPUT);
    pinMode(GPIO3, OUTPUT);

    digitalWrite(GPIO1, LOW);
    digitalWrite(GPIO2, LOW);
    digitalWrite(GPIO3, LOW);

    initWiFi();

    Serial.println();
    Serial.println("Initializing Firebase...");

    ssl_client.setInsecure();
    stream_ssl_client.setInsecure();

    initializeApp(
        aClient,
        app,
        getAuth(user_auth),
        processData,
        "authTask");

    Serial.println("Waiting for Firebase Authentication...");

    while (!app.ready())
    {
        app.loop();
        delay(100);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Firebase Authentication Success!");

    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);

    streamClient.setSSEFilters(
        "put,patch,keep-alive,cancel,auth_revoked");

    Database.get(
        streamClient,
        "/",
        processData,
        true,
        "streamTask");

    Serial.println("================================");
    Serial.println("Realtime Database Stream Started");
    Serial.println("Waiting For Database Events...");
    Serial.println("================================");
}
// ================= LOOP =================

void loop()
{
    app.loop();

    static unsigned long lastStatus = 0;

    if (millis() - lastStatus > 5000)
    {
        lastStatus = millis();

        Serial.println();
        Serial.println("========== SYSTEM STATUS ==========");

        Serial.print("WiFi : ");
        Serial.println(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED");

        Serial.print("IP : ");
        Serial.println(WiFi.localIP());

        Serial.print("RSSI : ");
        Serial.println(WiFi.RSSI());

        Serial.print("Heap : ");
        Serial.println(ESP.getFreeHeap());

        Serial.print("Firebase Ready : ");
        Serial.println(app.ready());

        Serial.println("===================================");
    }
}

// ================= FIREBASE CALLBACK =================

void processData(AsyncResult &aResult)
{
    if (!aResult.isResult())
        return;

    if (aResult.isError())
    {
        Serial.println();
        Serial.println("******** FIREBASE ERROR ********");

        Serial.printf(
            "Error: %s\n",
            aResult.error().message().c_str());

        Serial.printf(
            "Code : %d\n",
            aResult.error().code());

        Serial.println("*******************************");

        return;
    }

    if (!aResult.available())
        return;

    RealtimeDatabaseResult &RTDB =
        aResult.to<RealtimeDatabaseResult>();

    if (!RTDB.isStream())
        return;

    Serial.println();
    Serial.println("===== FIREBASE EVENT =====");

    Serial.printf(
        "Event : %s\n",
        RTDB.event().c_str());

    Serial.printf(
        "Path  : %s\n",
        RTDB.dataPath().c_str());

    Serial.printf(
        "Data  : %s\n",
        RTDB.to<String>().c_str());

    Serial.println("==========================");

    int dataType = RTDB.type();

    String fullPath = RTDB.dataPath();

    // Initial JSON Load

    if (dataType == 6)
    {
        DynamicJsonDocument doc(256);

        if (deserializeJson(doc,
                            RTDB.to<String>()) ==
            DeserializationError::Ok)
        {
            if (doc.containsKey("gpio1"))
            {
                int value = doc["gpio1"];
                digitalWrite(GPIO1, value ? HIGH : LOW);

                Serial.printf(
                    "GPIO1 -> %d\n",
                    value);
            }

            if (doc.containsKey("gpio2"))
            {
                int value = doc["gpio2"];
                digitalWrite(GPIO2, value ? HIGH : LOW);

                Serial.printf(
                    "GPIO2 -> %d\n",
                    value);
            }

            if (doc.containsKey("gpio3"))
            {
                int value = doc["gpio3"];
                digitalWrite(GPIO3, value ? HIGH : LOW);

                Serial.printf(
                    "GPIO3 -> %d\n",
                    value);
            }
        }

        return;
    }

    // Single GPIO Update

    int value = RTDB.to<int>();

    if (fullPath == "/gpio1")
    {
        digitalWrite(GPIO1,
                     value ? HIGH : LOW);

        Serial.printf(
            "GPIO1 Changed -> %d\n",
            value);
    }

    if (fullPath == "/gpio2")
    {
        digitalWrite(GPIO2,
                     value ? HIGH : LOW);

        Serial.printf(
            "GPIO2 Changed -> %d\n",
            value);
    }

    if (fullPath == "/gpio3")
    {
        digitalWrite(GPIO3,
                     value ? HIGH : LOW);

        Serial.printf(
            "GPIO3 Changed -> %d\n",
            value);
    }
}
