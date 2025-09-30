#ifndef _WEBAPI_H_
#define _WEBAPI_H_

#include <Arduino.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const uint8_t MAX_USERNAME_LENGTH = 8;

void webApiDebug(Stream &terminalStream);

AsyncWebServer& getWebServer();

void webApiSetup();
void webApiLoop();

#endif