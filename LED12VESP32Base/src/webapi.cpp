#include <webapi.h>
#include <wifi_handler.h>
#include <webinterface.h>
#include <config.h>
#include <device_state.h>
#include <ldr.h>

#include <Update.h>
#include <ESPmDNS.h>
#define U_PART U_SPIFFS

static const char *PrefSaveAsPreference = "sapr";
static const char *PrefSetLampState = "slst";
static unsigned long const REPORT_DELAY_MS = 5000;

Stream *debug_uart_web_interface = nullptr;

AsyncWebServer _server(80);
bool _serverStarted = false;
String _http_username;
String _http_password;

unsigned long _lastReport = 0;

AsyncWebServer& getWebServer() { return _server; }

size_t _otaContentLen;


void webApiDebug(Stream &terminalStream) { debug_uart_web_interface = &terminalStream; }

bool tryGetParam(AsyncWebServerRequest *request, const char *paramName, bool post, String &value)
{
  if (!request->hasParam(paramName, post))
    return false;
  value = request->getParam(paramName, post)->value();
  if (debug_uart_web_interface != nullptr)
  {
    debug_uart_web_interface->print(F("[API] found param "));
    debug_uart_web_interface->print(paramName);
    debug_uart_web_interface->print(F(" = "));
    debug_uart_web_interface->println(value);
  }
  return true;
}

/*

  Setting values via request parameters

*/

long limitValue(const String &rawValue, long maxVal)
{
  long value = rawValue.toInt();
  if (value < 0)
    return 0;
  return value > maxVal ? maxVal : value;
}

void parAllowNightLightMode(const String &rawValue, bool setAsPreference)
{
  bool value = rawValue.equals("true");
  // ignore everything but "true" or "false"
  if (value || rawValue.equals("false"))
  {
    if (!setAsPreference)
      modifyAllowNightLightMode(value);
    else if (value != allowNightLight())
      setAllowNightLight(value);
  }
}

void parMaxBrightness(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyMaxBrightness(value);
  else if (value != maxBrightness())
    setMaxBrightness(value);
}

void parMaxMovingTargetDistance(const String &rawValue, bool setAsPreference)
{
  uint16_t value = (uint16_t)limitValue(rawValue, UINT16_MAX);
  if (!setAsPreference)
    modifyMaxMovingTargetDistance(value);
  else if (value != maxMovingTargetDistance())
    setMaxMovingTargetDistance(value);
}

void parMaxMovingTargetEnergy(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyMaxMovingTargetEnergy(value);
  else if (value != maxMovingTargetEnergy())
    setMaxMovingTargetEnergy(value);
}

void parMaxNightLightBrightness(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyMaxNightLightBrightness(value);
  else if (value != maxNightLightBrightness())
    setMaxNightLightBrightness(value);
}

void parMaxStationaryTargetDistance(const String &rawValue, bool setAsPreference)
{
  uint16_t value = (uint16_t)limitValue(rawValue, UINT16_MAX);
  if (!setAsPreference)
    modifyMaxStationaryTargetDistance(value);
  else if (value != maxStationaryTargetDistance())
    setMaxStationaryTargetDistance(value);
}

void parMaxStationaryTargetEnergy(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyMaxStationaryTargetEnergy(value);
  else if (value != maxStationaryTargetEnergy())
    setMaxStationaryTargetEnergy(value);
}

void parMinMovingTargetDistance(const String &rawValue, bool setAsPreference)
{
  uint16_t value = (uint16_t)limitValue(rawValue, UINT16_MAX);
  if (!setAsPreference)
    modifyMinMovingTargetDistance(value);
  else if (value != minMovingTargetDistance())
    setMinMovingTargetDistance(value);
}

void parMinMovingTargetEnergy(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyMinStationaryTargetEnergy(value);
  else if (value != minStationaryTargetEnergy())
    setMinStationaryTargetEnergy(value);
}

void parMinStationaryTargetDistance(const String &rawValue, bool setAsPreference)
{
  uint16_t value = (uint16_t)limitValue(rawValue, UINT16_MAX);
  if (!setAsPreference)
    modifyMinStationaryTargetDistance(value);
  else if (value != minStationaryTargetDistance())
    setMinStationaryTargetDistance(value);
}

void parMinStationaryTargetEnergy(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyMinStationaryTargetEnergy(value);
  else if (value != minStationaryTargetEnergy())
    setMinStationaryTargetEnergy(value);
}

void parNightLightBrightness(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyNightLightBrightness(value);
  else if (value != nightLightBrightness())
    setNightLightBrightness(value);
}

void parNightLightLdrThreshold(const String &rawValue, bool setAsPreference)
{
  uint16_t value = (uint16_t)limitValue(rawValue, MAX_BRIGHTNESS);
  if (!setAsPreference)
    modifyNightLightThreshold(value);
  else if (value != nightLightThreshold())
    setNightLightThreshold(value);
}

void parOnBrightness(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyOnBrightness(value);
  else if (value != onBrightness())
    setOnBrightness(value);
}

void parNightLightOnDuration(const String &rawValue, bool setAsPreference)
{
  uint16_t value = (uint16_t)limitValue(rawValue, UINT16_MAX);
  if (!setAsPreference)
    modifyNightLightOnDurationSeconds(value);
  else if (value != nightLightOnDuration())
    setNightLightOnDuration(value);
}

void parBrightnessStep(const String &rawValue, bool setAsPreference)
{
  uint8_t value = (uint8_t)limitValue(rawValue, UINT8_MAX);
  if (!setAsPreference)
    modifyBrightnessStep(value);
  else if (value != brightnessStep())
    setBrightnessStep(value);
}

void parTransitionDurationMs(const String &rawValue, bool setAsPreference)
{
  uint16_t value = (uint16_t)limitValue(rawValue, UINT16_MAX);
  if (!setAsPreference)
    modifyTransitionDurationMs(value);
  else if (value != transitionDurationMs())
    setTransitionDurationMs(value);
}

void parWebAuthPassword(const String &rawValue)
{
  uint len = rawValue.length();
  if (len > 7 && len <= MAX_PASSPHRASE_LEN)
  {
    _http_password = rawValue;
    if (rawValue != getWebAuthPassword())
      setWebAuthPassword(rawValue);
  }
}

void parWebAuthUsername(const String &rawValue)
{
  uint len = rawValue.length();
  if (len > 3 && len <= MAX_USERNAME_LENGTH)
  {
    _http_username = rawValue;
    if (rawValue != getWebAuthUsername())
      setWebAuthUsername(rawValue);
  }
}

void parWifiApPassphrase(const String &rawValue)
{
  uint len = rawValue.length();
  if (len > 7 && len <= MAX_PASSPHRASE_LEN)
  {
    modifyApPassphrase(rawValue);
    // gets saved as preference when it has proved to work
  }
}

void parWifiApSsid(const String &rawValue)
{
  uint len = rawValue.length();
  if (len > 3 && len <= MAX_SSID_LEN)
  {
    modifyApSsid(rawValue);
    // gets saved as preference when it has proved to work
  }
}

void parWifiHostname(const String &rawValue)
{
  uint len = rawValue.length();
  if (len > 1 && len <= MAX_HOSTNAME_LEN)
  {
    modifyHostname(rawValue);
    // gets saved as preference when it has proved to work
  }
}

void parWifiStaPassphrase(const String &rawValue)
{
  uint len = rawValue.length();
  if (len > 7 && len <= MAX_PASSPHRASE_LEN)
  {
    modifyStaPassphrase(rawValue);
    if (rawValue != getWifiStaPassphrase())
      setWifiStaPassphrase(rawValue);
  }
}

void parWifiStaSsid(const String &rawValue)
{
  uint len = rawValue.length();
  if (len > 3 && len <= MAX_SSID_LEN)
  {
    modifyStaSsid(rawValue);
    if (rawValue != getWifiStaSsid())
      setWifiStaSsid(rawValue);
  }
}

void parSetLampState(const String &rawValue)
{
  bool value = rawValue.equals("true");
  // ignore everything but "true" or "false"
  if (value || rawValue.equals("false"))
  {
    modifyLightState(value);
  }
}

void toApiV1(AsyncWebServerRequest *request, bool isPost)
{
  Serial.println(isPost ? F("POST"): F("GET"));
  int params = request->params();
  for (int i = 0; i < params; i++) {
    const AsyncWebParameter* p = request->getParam(i);
    Serial.printf("%s: %s\n", p->name().c_str(), p->value().c_str());
  }
  /*
  if(!request->authenticate(_http_username.c_str(), _http_password.c_str()))
      return request->requestAuthentication();
  */
  String rawValue;
  bool saveAsPreference = tryGetParam(request, PrefSaveAsPreference, isPost, rawValue) && rawValue.equals("true");
  if (tryGetParam(request, PrefAllowNightLight, isPost, rawValue))
    parAllowNightLightMode(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefBrightnessStep, isPost, rawValue))
    parBrightnessStep(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxBrightness, isPost, rawValue))
    parMaxBrightness(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxMovingTargetDistance, isPost, rawValue))
    parMaxMovingTargetDistance(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxMovingTargetEnergy, isPost, rawValue))
    parMaxMovingTargetEnergy(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxNightLightBrightness, isPost, rawValue))
    parMaxNightLightBrightness(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxStationaryTargetDistance, isPost, rawValue))
    parMaxStationaryTargetDistance(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxStationaryTargetEnergy, isPost, rawValue))
    parMaxStationaryTargetEnergy(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinMovingTargetDistance, isPost, rawValue))
    parMinMovingTargetDistance(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinMovingTargetEnergy, isPost, rawValue))
    parMinMovingTargetEnergy(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinStationaryTargetDistance, isPost, rawValue))
    parMinStationaryTargetDistance(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinStationaryTargetEnergy, isPost, rawValue))
    parMinStationaryTargetEnergy(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefNightLightBrightness, isPost, rawValue))
    parNightLightBrightness(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefNightLightLdrThreshold, isPost, rawValue))
    parNightLightLdrThreshold(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefOnBrightness, isPost, rawValue))
    parOnBrightness(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefNightLightOnDuration, isPost, rawValue))
    parNightLightOnDuration(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefTransitionDurationMs, isPost, rawValue))
    parTransitionDurationMs(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefWebAuthPassword, isPost, rawValue))
    parWebAuthPassword(rawValue);
  if (tryGetParam(request, PrefWebAuthUsername, isPost, rawValue))
    parWebAuthUsername(rawValue);
  /*
  if (tryGetParam(request, PrefWifiApIpAddress, isPost, rawValue))
    parWifiApIpAddress(rawValue);
  if (tryGetParam(request, PrefWifiApNetmask, isPost, rawValue))
    parWifiApNetmask(rawValue);
  */
  if (tryGetParam(request, PrefWifiApPassphrase, isPost, rawValue))
    parWifiApPassphrase(rawValue);
  if (tryGetParam(request, PrefWifiApSsid, isPost, rawValue))
    parWifiApSsid(rawValue);
  if (tryGetParam(request, PrefWifiHostname, isPost, rawValue))
    parWifiHostname(rawValue);
  if (tryGetParam(request, PrefWifiStaPassphrase, isPost, rawValue))
    parWifiStaPassphrase(rawValue);
  if (tryGetParam(request, PrefWifiStaSsid, isPost, rawValue))
    parWifiStaSsid(rawValue);
  if (tryGetParam(request, PrefSetLampState, isPost, rawValue))
    parSetLampState(rawValue);
}

void toApiV1Post(AsyncWebServerRequest *request)
{
  toApiV1(request, true);
  request->send(200, "text/plain", "Hello, POST: ");
}

void toApiV1Get(AsyncWebServerRequest *request)
{
  toApiV1(request, false);
  request->send(200, "text/plain", "Hello, GET: ");
}

void handleUpdate(AsyncWebServerRequest *request)
{
  const char *html = "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  request->send(200, "text/html", html);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    Serial.println("Update");
    _otaContentLen = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
    {
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
  }

  if (final)
  {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "20");
    response->addHeader("Location", "/");
    request->send(response);
    if (!Update.end(true))
    {
      Update.printError(Serial);
    }
    else
    {
      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

void printProgress(size_t prg, size_t sz) {
  Serial.printf("Progress: %d%%\n", (prg*100)/_otaContentLen);
}

void addWebApiHandlers(AsyncWebServer &server)
{
  // Send a GET request to <IP>/get?message=<message>
  server.on("/v1/get", HTTP_GET, toApiV1Get);
  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/v1/post", HTTP_POST, toApiV1Post);

  // OTA
  server.on("/update", HTTP_GET, handleUpdate);
  server.on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleDoUpdate);

  Update.onProgress(printProgress);
}

void webApiSetup()
{
  addWebApiHandlers(_server);
  addWebInterfaceHandlers(_server);
  _http_password = getWebAuthPassword();
  _http_username = getWebAuthUsername();
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
