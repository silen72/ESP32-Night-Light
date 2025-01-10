#ifndef _WEBAPI_H_
#define _WEBAPI_H_

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer getWebServer();

void webApiDebug(Stream &terminalStream);

void webApiSetup();
void webApiLoop();


#endif