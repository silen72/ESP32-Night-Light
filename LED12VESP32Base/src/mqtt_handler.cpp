#include "mqtt_handler.h"

String _mqtt_server;
String _mqtt_user;
String _mqtt_password;

void modifyMqttServer(const String &value)
{
    _mqtt_server = value;
}

void modifyMqttUsername(const String &value)
{
    _mqtt_user = value;
}

void modifyMqttPassword(const String &value)
{
    _mqtt_password = value;
}