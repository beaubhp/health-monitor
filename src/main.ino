// This is the main file for the program. The code below does the following:
// 1. Connects to wifi
// 2. Retrieves current time
// 3. Setup connect to IoT hub 
// 4. Sends message to IoT hub

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

#include "config.h"

static bool messagePending = false;
static bool messageSending = true;

static int interval = INTERVAL;

void blinkLED() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

void initWifi() {
    // Attempt to connect to Wifi network:
    Serial.printf("Attempting to connect to WIFI_SSID : %s.\r\n", WIFI_SSID );

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(WIFI_SSID , WIFI_PASS );
    while (WiFi.status() != WL_CONNECTED) {
        // Get Mac Address and show it.
        // WiFi.macAddress(mac) save the mac address into a six length array, but the endian may be different. The huzzah board should
        // start from mac[0] to mac[5], but some other kinds of board run in the oppsite direction.
        uint8_t mac[6];
        WiFi.macAddress(mac);
        Serial.printf("You device with MAC address %02x:%02x:%02x:%02x:%02x:%02x connects to %s failed! Waiting 10 seconds to retry.\r\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], WIFI_SSID );
        WiFi.begin(WIFI_SSID , WIFI_PASS );
        delay(10000);
    }
    Serial.printf("Connected to wifi %s.\r\n", WIFI_SSID);
}

void initTime() {
    time_t epochTime;
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true) {
        epochTime = time(NULL);

        if (epochTime == 0) {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        } else {
            Serial.printf("Fetched NTP epoch time is: %lu.\r\n", epochTime);
            break;
        }
    }
}

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);

    initWifi();
    initTime();
    initSensor();

    iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(CONNECTION_STRING, MQTT_Protocol);
    if (iotHubClientHandle == NULL) {
        Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
        while (1);
    }

    IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "HappyPath_AdafruitFeatherHuzzah-C");
    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
    IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
    IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);
}

static int messageCount = 1;


void loop() {

    int heartValue = analogRead(HEARTPIN);
    Serial.println("HR Recieved (<100)");

    if (heartValue > 100 && (!messagePending && messageSending)) {
        Serial.println("Heart rate has passed parameter: 100");
        char messagePayload[MESSAGE_MAX_LEN];
        bool heartrateAlert = readMessage(messageCount, messagePayload);
        sendMessage(iotHubClientHandle, messagePayload, heartrateAlert);
        messageCount++;
    }
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    delay(interval);
}
