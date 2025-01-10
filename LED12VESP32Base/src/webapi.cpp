#include "webapi.h"
#include "wifi_handler.h"
#include "webinterface.h"

Stream *debug_uart_web_interface = nullptr;

AsyncWebServer _server(80);
bool _serverStarted = false;

unsigned long _lastReport = 0;
unsigned long const REPORT_DELAY_MS = 5000;

AsyncWebServer getWebServer() { return _server; }

void webApiDebug(Stream &terminalStream) { debug_uart_web_interface = &terminalStream; }

void webApiSetup()
{
  addHandlers(_server);
}

void webApiLoop()
{
  WifiStateInfo wifiInfo = wifiCurrentState();
  if (!_serverStarted && wifiInfo.mode == WifiMode::WifiMode_AP && wifiInfo.currentState == WifiState::AP_OK)
  {
    if (debug_uart_web_interface != nullptr)
    {
      debug_uart_web_interface->println(F("[API] start AsyncWebServer"));
    }
    _server.begin();
    if (debug_uart_web_interface != nullptr)
    {
      debug_uart_web_interface->println(F("[API] AsyncWebServer started"));
    }
    _serverStarted = true;
  }
  if (!_serverStarted && millis() - _lastReport >= REPORT_DELAY_MS)
  {
    debug_uart_web_interface->println(F("[API] AsyncWebServer not started yet"));
    debug_uart_web_interface->println(F("WifiStateInfo:"));
    debug_uart_web_interface->print(F(" WifiMode: "));
    debugPrintWifiMode(debug_uart_web_interface, wifiInfo.mode, true);
    debug_uart_web_interface->print(F(" AP ModeResult: "));
    debugPrintModeResult(debug_uart_web_interface, wifiInfo.apModeResult, true);
    debug_uart_web_interface->print(F(" STA ModeResult: "));
    debugPrintModeResult(debug_uart_web_interface, wifiInfo.staModeResult, true);
    debug_uart_web_interface->print(F(" WifiState: "));
    debugPrintWifiState(debug_uart_web_interface, wifiInfo.currentState, true);
    _lastReport = millis();
  }
}
