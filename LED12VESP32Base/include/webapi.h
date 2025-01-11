#ifndef _WEBAPI_H_
#define _WEBAPI_H_

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

void webApiDebug(Stream &terminalStream);

AsyncWebServer getWebServer();

void webApiSetup();
void webApiLoop();

#endif