// The code below does the following:
// 1. Sends data to IoT hub and reads data for conditional statement

#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>

void initSensor() {
    // use HUZZAH_DATA, if using other sensors, use DHT or external libraries.
}

float readHeartrate() {
    return random(20,30);
}

float readID() {
    return random(1,10);
}

bool readMessage(int messageId, char *payload) {
    float heartrate = readHeartrate();
    float identicator = readID();
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
    bool heartrateAlert = false;

    // NAN is not the valid json, change it to NULL
    if (std::isnan(heartrate)) {
        root["heartrate"] = NULL;
    } else {
        root["heartrate"] = heartrate;
        if (heartrate > HEARTRATE_ALERT) {
            heartrateAlert = true;
        }
    }

    if (std::isnan(identicator)) {
        root["identicator"] = NULL;
    } else {
        root["identicator"] = identicator;
    }
    root.printTo(payload, MESSAGE_MAX_LEN);
    return heartrateAlert;
}

void parseTwinMessage(char *message) {
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(message);
    if (!root.success()) {
        Serial.printf("Parse %s failed.\r\n", message);
        return;
    }

    if (root["desired"]["interval"].success()) {
        interval = root["desired"]["interval"];
    } else if (root.containsKey("interval")) {
        interval = root["interval"];
    }
}
