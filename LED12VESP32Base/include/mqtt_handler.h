#ifndef _MQTTHANDLER_H_
    #define _MQTTHANDLER_H_

    #include <Arduino.h>

    #define MAX_MQTT_SERVER_LENGTH 64
    #define MAX_MQTT_USERNAME_LENGTH 12
    #define MAX_MQTT_PASSWORD_LENGTH 24

    void modifyMqttServer(const String &value);
    void modifyMqttUsername(const String &value);
    void modifyMqttPassword(const String &value);

#endif