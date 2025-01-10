#ifndef _WEBINERFACE_H_
#define _WEBINERFACE_H_

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


void webInterfaceDebug(Stream &terminalStream);

void webInterfaceSetup();
void webInterfaceLoop();

void addHandlers(AsyncWebServer &server);

#endif