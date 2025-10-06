#include <webapi.h>
#include <wifi_handler.h>
#include <webinterface.h>
#include <config.h>
#include <device_state.h>
#include <ldr.h>

#include <Update.h>
#include <ESPmDNS.h>
#include <mqtt_handler.h>

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

AsyncWebServer &getWebServer() { return _server; }

size_t _otaContentLen;

struct boundL_t
{
  bool isNumber = false;
  long rawValueL = 0;
  long boundValueL = 0;
  double rawValueD = 0;
};

struct bound8_t : boundL_t
{
  uint8_t value = 0;
};

struct bound16_t : boundL_t
{
  uint16_t value = 0;
};

struct convertedBool
{
  bool isBool = false;
  bool value = false;
};

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

/// @brief Bounds a value to a value between 0 and a maximum value
/// @param rawValue The value to bound
/// @param maxVal The allowed maximum value
// void boundValue(const String &rawValue, long maxVal, boundL_t &convertInfo) { boundValue(rawValue, maxVal, 0, convertInfo); }

/// @brief Bounds a value to a value between a minimum and a maximum value
/// @param rawValue The value to bound
/// @param maxVal The allowed maximum value
/// @param minVal The allowed minimum value
void boundValue(const String &rawValue, long minVal, long maxVal, boundL_t &convertInfo)
{
  char *p;
  convertInfo.rawValueD = strtod(rawValue.c_str(), &p);
  convertInfo.isNumber = *p == 0;
  convertInfo.rawValueL = rawValue.toInt();
  long value = convertInfo.rawValueL;
  if (value < minVal)
    value = minVal;
  else if (value > maxVal)
    value = maxVal;
  convertInfo.boundValueL = value;
}

/// @brief Converts and bounds a number contained in a String to the uint8_t range min..max
/// @param rawValue The String containing a number
/// @param min
/// @param max
/// @param result The result of conversion and bounding
void boundValue8_t(const String &rawValue, uint8_t min, uint8_t max, bound8_t &result)
{
  boundValue(rawValue, min, max, result);
  result.value = (uint8_t)result.boundValueL;
}

/// @brief Converts and bounds a number contained in a String to the uint8_t range 0..UINT8_MAX
/// @param rawValue The String containing a number
/// @param result The result of conversion and bounding
void boundValue8_t(const String &rawValue, bound8_t &result) { boundValue8_t(rawValue, 0, UINT8_MAX, result); }

/// @brief Converts and bounds a number contained in a String to the uint16_t range min..max
/// @param rawValue The String containing a number
/// @param min
/// @param max
/// @param result The result of conversion and bounding
void boundValue16_t(const String &rawValue, uint16_t min, uint16_t max, bound16_t &result)
{
  boundValue(rawValue, min, max, result);
  result.value = (uint16_t)result.boundValueL;
}

/// @brief Converts and bounds a number contained in a String to the uint16_t 0..UINT16_MAX
/// @param rawValue The String containing a number
/// @param result The result of conversion and bounding
void boundValue16_t(const String &rawValue, bound16_t &result) { boundValue16_t(rawValue, 0, UINT16_MAX, result); }

/// @brief checks whether a String is within length boundaries
/// @param rawValue The String to check
/// @param min minimum length (inclusive)
/// @param max maximum length (inclusive)
/// @return whether the rawValues length is in between min (>=) and max (<=)
bool withinLength(const String &rawValue, size_t min, size_t max)
{
  uint len = rawValue.length();
  return len >= min && len <= max;
}

/// @brief Converts a boolean value contained in a String
/// @param rawValue The String containing a boolean value
/// @param convInfo The result of the conversion
void toBool(const String &rawValue, convertedBool &convInfo)
{
  convInfo.value = rawValue.equalsIgnoreCase(F("true"));
  convInfo.isBool = convInfo.value || rawValue.equalsIgnoreCase(F("false"));
}

void parAllowNightLightMode(const String &rawValue, bool setAsPreference)
{
  convertedBool cb;
  toBool(rawValue, cb);
  if (cb.isBool)
    (setAsPreference ? setAllowNightLight : modifyAllowNightLightMode)(cb.value);
}

void parMaxBrightness(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMaxBrightness : modifyMaxBrightness)(bv.value);
}

void parMaxMovingTargetDistance(const String &rawValue, bool setAsPreference)
{
  bound16_t bv;
  boundValue16_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMaxMovingTargetDistance : modifyMaxMovingTargetDistance)(bv.value);
}

void parMaxMovingTargetEnergy(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMaxMovingTargetEnergy : modifyMaxMovingTargetEnergy)(bv.value);
}

void parMaxNightLightBrightness(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMaxNightLightBrightness : modifyMaxNightLightBrightness)(bv.value);
}

void parMaxStationaryTargetDistance(const String &rawValue, bool setAsPreference)
{
  bound16_t bv;
  boundValue16_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMaxStationaryTargetDistance : modifyMaxStationaryTargetDistance)(bv.value);
}

void parMaxStationaryTargetEnergy(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMaxStationaryTargetEnergy : modifyMaxStationaryTargetEnergy)(bv.value);
}

void parMinMovingTargetDistance(const String &rawValue, bool setAsPreference)
{
  bound16_t bv;
  boundValue16_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMinMovingTargetDistance : modifyMinMovingTargetDistance)(bv.value);
}

void parMinMovingTargetEnergy(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMinStationaryTargetEnergy : modifyMinStationaryTargetEnergy)(bv.value);
}

void parMinStationaryTargetDistance(const String &rawValue, bool setAsPreference)
{
  bound16_t bv;
  boundValue16_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMinStationaryTargetDistance : modifyMinStationaryTargetDistance)(bv.value);
}

void parMinStationaryTargetEnergy(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setMinStationaryTargetEnergy : modifyMinStationaryTargetEnergy)(bv.value);
}

void parNightLightBrightness(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setNightLightBrightness : modifyNightLightBrightness)(bv.value);
}

void parNightLightLdrThreshold(const String &rawValue, bool setAsPreference)
{
  bound16_t bv;
  boundValue16_t(rawValue, 0, MAX_BRIGHTNESS, bv);
  if (bv.isNumber)
    (setAsPreference ? setNightLightThreshold : modifyNightLightThreshold)(bv.value);
}

void parOnBrightness(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue8_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setOnBrightness : modifyOnBrightness)(bv.value);
}

void parNightLightOnDuration(const String &rawValue, bool setAsPreference)
{
  bound16_t bv;
  boundValue16_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setNightLightOnDuration : modifyNightLightOnDurationSeconds)(bv.value);
}

void parBrightnessStep(const String &rawValue, bool setAsPreference)
{
  bound8_t bv;
  boundValue(rawValue, 1, UINT8_MAX, bv);
  if (bv.isNumber)
    (setAsPreference ? setBrightnessStep : modifyBrightnessStep)(bv.value);
}

void parTransitionDurationMs(const String &rawValue, bool setAsPreference)
{
  bound16_t bv;
  boundValue16_t(rawValue, bv);
  if (bv.isNumber)
    (setAsPreference ? setTransitionDurationMs : modifyTransitionDurationMs)(bv.value);
}

void parWebAuthPassword(const String &rawValue)
{
  if (!withinLength(rawValue, 8, MAX_PASSPHRASE_LEN))
    return;
  _http_password = rawValue;
  setWebAuthPassword(rawValue); // also save as preference
}

void parWebAuthUsername(const String &rawValue)
{
  if (!withinLength(rawValue, 4, MAX_USERNAME_LENGTH))
    return;
  _http_username = rawValue;
  setWebAuthUsername(rawValue); // also save as preference
}

void parWifiApPassphrase(const String &rawValue)
{
  if (withinLength(rawValue, 8, MAX_PASSPHRASE_LEN))
    modifyApPassphrase(rawValue); // gets saved as preference when it has proved to work
}

void parWifiApIpAddress(const String &rawValue)
{
  if (withinLength(rawValue, IP_LENGTH_MIN, IP_LENGTH_MAX))
    modifyApIpAddress(rawValue); // gets saved as preference when it has proved to work
}

void parWifiApNetmask(const String &rawValue)
{
  if (withinLength(rawValue, IP_LENGTH_MIN, IP_LENGTH_MAX))
    modifyApIpNetmask(rawValue); // gets saved as preference when it has proved to work
}

void parWifiApSsid(const String &rawValue)
{
  if (withinLength(rawValue, 4, MAX_SSID_LEN))
    modifyApSsid(rawValue); // gets saved as preference when it has proved to work
}

void parWifiHostname(const String &rawValue)
{
  if (withinLength(rawValue, 2, MAX_HOSTNAME_LEN))
    modifyHostname(rawValue); // gets saved as preference when it has proved to work
}

void parWifiStaPassphrase(const String &rawValue)
{
  if (withinLength(rawValue, 8, MAX_PASSPHRASE_LEN))
  {
    modifyStaPassphrase(rawValue);
    if (rawValue != getWifiStaPassphrase())
      setWifiStaPassphrase(rawValue);
  }
}

void parWifiStaSsid(const String &rawValue)
{
  if (withinLength(rawValue, 4, MAX_SSID_LEN))
  {
    modifyStaSsid(rawValue);
    if (rawValue != getWifiStaSsid())
      setWifiStaSsid(rawValue);
  }
}

void parMqttServer(const String &rawValue)
{
  if (withinLength(rawValue, 4, MAX_MQTT_SERVER_LENGTH))
  {
    modifyMqttServer(rawValue);
    if (rawValue != getMqttServer())
      setMqttServer(rawValue);
  }
}

void parMqttUser(const String &rawValue)
{
  if (withinLength(rawValue, 0, MAX_MQTT_USERNAME_LENGTH))
  {
    modifyMqttUsername(rawValue);
    if (rawValue != getMqttUsername())
      setMqttUsername(rawValue);
  }
}

void parMqttPassword(const String &rawValue)
{
  if (withinLength(rawValue, 0, MAX_MQTT_PASSWORD_LENGTH))
    modifyMqttPassword(rawValue);
}

void parSetLampState(const String &rawValue)
{
  convertedBool cb;
  toBool(rawValue, cb);
  if (cb.isBool)
    modifyLightState(cb.value);
}

void toApiV1(AsyncWebServerRequest *request, bool isPost)
{
  Serial.println(isPost ? F("POST") : F("GET"));
  int params = request->params();
  for (int i = 0; i < params; i++)
  {
    const AsyncWebParameter *p = request->getParam(i);
    Serial.printf("%s: %s\n", p->name().c_str(), p->value().c_str());
  }
  /*
  if(!request->authenticate(_http_username.c_str(), _http_password.c_str()))
      return request->requestAuthentication();
  */
  String rawValue;
  bool saveAsPreference = tryGetParam(request, PrefSaveAsPreference, isPost, rawValue) && rawValue.equals("true");

  /*
  Light
  */
  if (tryGetParam(request, PrefOnBrightness, isPost, rawValue))
    parOnBrightness(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxBrightness, isPost, rawValue))
    parMaxBrightness(rawValue, saveAsPreference);

  /*
  Nightlight
  */
  if (tryGetParam(request, PrefAllowNightLight, isPost, rawValue))
    parAllowNightLightMode(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefNightLightBrightness, isPost, rawValue))
    parNightLightBrightness(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxNightLightBrightness, isPost, rawValue))
    parMaxNightLightBrightness(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefNightLightOnDuration, isPost, rawValue))
    parNightLightOnDuration(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefNightLightLdrThreshold, isPost, rawValue))
    parNightLightLdrThreshold(rawValue, saveAsPreference);

  /*
  Presence
  */
  if (tryGetParam(request, PrefMaxMovingTargetDistance, isPost, rawValue))
    parMaxMovingTargetDistance(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinMovingTargetDistance, isPost, rawValue))
    parMinMovingTargetDistance(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxStationaryTargetDistance, isPost, rawValue))
    parMaxStationaryTargetDistance(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinStationaryTargetDistance, isPost, rawValue))
    parMinStationaryTargetDistance(rawValue, saveAsPreference);

  if (tryGetParam(request, PrefMaxMovingTargetEnergy, isPost, rawValue))
    parMaxMovingTargetEnergy(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinMovingTargetEnergy, isPost, rawValue))
    parMinMovingTargetEnergy(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMaxStationaryTargetEnergy, isPost, rawValue))
    parMaxStationaryTargetEnergy(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefMinStationaryTargetEnergy, isPost, rawValue))
    parMinStationaryTargetEnergy(rawValue, saveAsPreference);

  /*
  Network
  */
  // Web interface
  if (tryGetParam(request, PrefWebAuthUsername, isPost, rawValue))
    parWebAuthUsername(rawValue);
  if (tryGetParam(request, PrefWebAuthPassword, isPost, rawValue))
    parWebAuthPassword(rawValue);

  // WiFi Access
  if (tryGetParam(request, PrefWifiStaSsid, isPost, rawValue))
    parWifiStaSsid(rawValue);
  if (tryGetParam(request, PrefWifiStaPassphrase, isPost, rawValue))
    parWifiStaPassphrase(rawValue);
  if (tryGetParam(request, PrefWifiHostname, isPost, rawValue))
    parWifiHostname(rawValue);

  // Access Point
  if (tryGetParam(request, PrefWifiApSsid, isPost, rawValue))
    parWifiApSsid(rawValue);
  if (tryGetParam(request, PrefWifiApPassphrase, isPost, rawValue))
    parWifiApPassphrase(rawValue);
  if (tryGetParam(request, PrefWifiApIpAddress, isPost, rawValue))
    parWifiApIpAddress(rawValue);
  if (tryGetParam(request, PrefWifiApNetmask, isPost, rawValue))
    parWifiApNetmask(rawValue);

  // MQTT
  if (tryGetParam(request, PrefMqttServer, isPost, rawValue))
    parMqttServer(rawValue);
  if (tryGetParam(request, PrefMqttUser, isPost, rawValue))
    parMqttUser(rawValue);
  if (tryGetParam(request, PrefMqttPassword, isPost, rawValue))
    parMqttPassword(rawValue);

  /*
  System
  */
  if (tryGetParam(request, PrefTransitionDurationMs, isPost, rawValue))
    parTransitionDurationMs(rawValue, saveAsPreference);
  if (tryGetParam(request, PrefBrightnessStep, isPost, rawValue))
    parBrightnessStep(rawValue, saveAsPreference);

  /*
  Actions
  */
  if (tryGetParam(request, PrefSetLampState, isPost, rawValue))
    parSetLampState(rawValue);
  // PrefSaveAsPreference
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

void printProgress(size_t prg, size_t sz)
{
  Serial.printf("Progress: %d%%\n", (prg * 100) / _otaContentLen);
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
