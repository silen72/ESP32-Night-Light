#include <webinterface.h>
#include <wifi_handler.h>
#include <device_state.h>
#include <device_common.h>

Stream *debug_uart_web_api = nullptr;

// the css for all web pages, minified (-> https://www.toptal.com/developers/cssminifier)
const char style_css[] = PROGMEM R"rawliteral(
h1,h2,h3{color:#fae1d2}html{font-family:Arial;display:inline-block;text-align:center}h1{font-size:2rem}h2{font-size:1.5rem}h3{font-size:1.2rem}label,p{font-size:1rem;color:#e1e1e1}input[type=number]{font-size:1rem;color:#000}button{padding:.4em .8em;background:#08173f;border:thin solid #1c4eda;color:#fff;text-shadow:0 -.05em .05em #333;font-size:125%;line-height:1.5}body{max-width:600px;margin:0 auto;padding-bottom:25px;background-color:#133592}
)rawliteral";

// the inital web site (see index.html), minified (-> https://htmlminifier.com/)
const char index_html[] = PROGMEM R"rawliteral(
<!doctypehtml><title>ESP32 LED Strip</title><meta content="width=device-width,initial-scale=1"name="viewport"><link href="style.css"rel="stylesheet"><link href="data:,"rel="icon"><body><script>function configButton(){setTimeout(function(){window.open("config.html","_self")},300)}</script><h1>ESP32 LED Strip</h1><p>Firmware: %FW_MA%.%FW_MI%.%FW_P%<h2>Light is %LIGHT_STATE% ( %LIGHT_BRIGHT% )</h2><h2>Night Light is %NIGHT_LIGHT_STATE% ( %NIGHT_LIGHT_BRIGHT% )</h2><p>LDR: %LDR%, thres %LDR_THRSH%<p>Motion: %PRES_MOT%<p>distance: %PRES_MOT_DIST% , min %PRES_MOT_DIST_MIN%, max %PRES_MOT_DIST_MAX%<p>energy: %PRES_MOT_ENER% , min %PRES_MOT_ENER_MIN%, max %PRES_MOT_ENER_MAX%<p>Presence (stationary): %PRES_STAT%<p>distance: %PRES_STAT_DIST% , min %PRES_STAT_DIST_MIN%, max %PRES_STAT_DIST_MAX%<p>energy: %PRES_STAT_ENER% , min %PRES_STAT_ENER_MIN%, max %PRES_STAT_ENER_MAX%<p>No Presence duration: %PRES_NON_DUR% , max %PRES_NON_DUR_MAX%</p><button onclick="configButton()">Configuration</button>
)rawliteral";

// the configuration web site (see config.html), minified (-> https://htmlminifier.com/)
const char config_html[] = PROGMEM R"rawliteral(
<!doctypehtml><title>ESP32 LED Strip Configuration</title><meta content="width=device-width,initial-scale=1"name="viewport"><link href="style.css"rel="stylesheet"><link href="data:,"rel="icon"><h1>Configuration</h1><h2>Light</h2><p>Lower values mean lower brightness. Allowed values: 1..255.<form action="/v1/post"method="post"><label for="obr">On Brightness:</label> <input id="obr"name="obr"type="number"value="210"max="210"min="1"> <button name="bobr"value="1">Set</button></form><form action="/v1/post"method="post"><label for="mbr">Maximum Brightness:</label> <input id="mbr"name="mbr"type="number"value="210"max="255"min="1"> <button name="bmbr"value="1">Set</button></form><h2>Night Light</h2><form action="/v1/post"method="post"><label for="alnl">Allow Night Light:</label> <input id="alnl"name="alnl"type="checkbox"checked> <button name="balnl"value="1">Set</button></form><form action="/v1/post"method="post"><label for="nlbr">Brightness:</label> <input id="nlbr"name="nlbr"type="range"value="5"> <button name="bnlbr"value="1">Set</button></form><form action="/v1/post"method="post"><label for="odu">On duration (seconds):</label> <input id="odu"name="odu"type="number"value="30"max="600"min="1"> <button name="bodu"value="1">Set</button></form><h2>Brightness detection</h2><p>Lower values mean lower brightness. Allowed values: 1..4095.<form action="/v1/post"method="post"><label for="nllt">LDR Threshold:</label> <input id="nllt"name="nllt"type="number"value="30"max="4095"min="1"> <button name="bnllt"value="1">Set</button></form><h2>Presence detection</h2><p>Allowed distance values: 30 .. 800.<h3>Movement detection</h3><form action="/v1/post"method="post"><label for="mimd">Minimum distance (cm):</label> <input id="mimd"name="mimd"type="number"value="30"max="500"min="30"> <button name="bmimd"value="1">Set</button></form><form action="/v1/post"method="post"><label for="mamd">Maximum distance (cm):</label> <input id="mamd"name="mamd"type="number"value="500"max="500"min="30"> <button name="bmamd"value="1">Set</button></form><form action="/v1/post"method="post"><label for="mime">Minimum energy (%):</label> <input id="mime"name="mime"type="number"value="0"max="100"min="0"> <button name="bmime"value="1">Set</button></form><form action="/v1/post"method="post"><label for="mame">Maximum energy (%):</label> <input id="mame"name="mame"type="number"value="100"max="100"min="0"> <button name="bmame"value="1">Set</button></form><h3>Stationary detection</h3><h2>Web interface</h2><h2>WiFi</h2><h2>Access Point</h2>
)rawliteral";

String localIPURL()
{
  WifiStateInfo wifiInfo = wifiCurrentState();
  return "http://" + wifiInfo.address.toString(); // a string version of the local IP with http, used for redirecting clients to your webpage
}

void webInterfaceDebug(Stream &terminalStream) { debug_uart_web_api = &terminalStream; }

// replaces placeholders in index_html
String processorConfig(const String &var)
{
  return String();
}

// replaces placeholders in index_html
String processorIndex(const String &var)
{
  DeviceStateInfo info = getDeviceState();
  bool isOn = info.state == State::START_TRANSIT_TO_ON || info.state == State::TRANSIT_TO_ON || info.state == State::ON;
  bool isNightLightOn = !isOn && (info.state == State::START_TRANSIT_TO_NIGHT_LIGHT || info.state == State::TRANSIT_TO_NIGHT_LIGHT || info.state == State::NIGHT_LIGHT_ON);

  if (var == "FW_MA")
  {
    return String(FIRMWARE_VERSION_MAJOR);
  }
  if (var == "FW_MI")
  {
    return String(FIRMWARE_VERSION_MINOR);
  }
  if (var == "FW_P")
  {
    return String(FIRMWARE_VERSION_PATCH);
  }
  if (var == "LIGHT_STATE")
  {
    return isOn ? "ON" : "OFF";
  }
  if (var == "LIGHT_BRIGHT")
  {
    return isOn ? String(info.onBrightness) : "0";
  }
  if (var == "NIGHT_LIGHT_STATE")
  {
    return String(isNightLightOn ? "ON" : "OFF") + String(isNightLightOn ? "" : String(info.allowNightLightMode ? " (enabled)" : " (disabled)"));
  }
  if (var == "NIGHT_LIGHT_BRIGHT")
  {
    return isNightLightOn ? String(info.nightLightBrightness) : "0";
  }
  if (var == "LDR")
  {
    return String(info.ldrValue);
  }
  if (var == "LDR_THRSH")
  {
    return String(info.nightLightThreshold);
  }
  if (var == "PRES_MOT")
  {
    return info.movingTargetDetected ? "YES" : "NO";
  }
  if (var == "PRES_MOT_DIST")
  {
    return String(info.movingTargetDistance);
  }
  if (var == "PRES_MOT_DIST_MIN")
  {
    return String(info.movingTargetDistanceMin);
  }
  if (var == "PRES_MOT_DIST_MAX")
  {
    return String(info.movingTargetDistanceMax);
  }
  if (var == "PRES_MOT_ENER")
  {
    return String(info.movingTargetEnergy);
  }
  if (var == "PRES_MOT_ENER_MIN")
  {
    return String(info.movingTargetEnergyMin);
  }
  if (var == "PRES_MOT_ENER_MAX")
  {
    return String(info.movingTargetEnergyMax);
  }
  if (var == "PRES_STAT")
  {
    return info.stationaryTargetDetected ? "YES" : "NO";
  }
  if (var == "PRES_STAT_DIST")
  {
    return String(info.stationaryTargetDistance);
  }
  if (var == "PRES_STAT_DIST_MIN")
  {
    return String(info.stationaryTargetDistanceMin);
  }
  if (var == "PRES_STAT_DIST_MAX")
  {
    return String(info.stationaryTargetDistanceMax);
  }
  if (var == "PRES_STAT_ENER")
  {
    return String(info.stationaryTargetEnergy);
  }
  if (var == "PRES_STAT_ENER_MIN")
  {
    return String(info.stationaryTargetEnergyMin);
  }
  if (var == "PRES_STAT_ENER_MAX")
  {
    return String(info.stationaryTargetEnergyMax);
  }
  if (var == "PRES_NON_DUR")
  {
    return String(info.noPresenceDuration);
  }
  if (var == "PRES_NON_DUR_MAX")
  {
    return String(info.nightLightOnDuration);
  }
  return String();
}

void webInterfaceSetup()
{
}

void webInterfaceLoop()
{
}

/*

  Web request handlers

*/

void toCss(AsyncWebServerRequest *request) { request->send(200, "text/css", style_css); }
void toOK(AsyncWebServerRequest *request) { request->send(200); }
void toNotFound(AsyncWebServerRequest *request) { request->send(404); }
void toLogout(AsyncWebServerRequest *request) { request->redirect("http://logout.net"); }
void toPortal(AsyncWebServerRequest *request)
{
  const String redirectToUrl = localIPURL();
  Serial.print(F("requested URL: 'http://"));
  Serial.print(request->host());
  Serial.print(request->url());
  Serial.println("', redirected to " + redirectToUrl);
  request->redirect(redirectToUrl);
}
void toMainPage(AsyncWebServerRequest *request)
{
  Serial.println(F("Serve Basic HTML Page: start"));
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html, processorIndex);
  response->addHeader("Cache-Control", "public,max-age=2000"); // cache the site this duration (refreshing overrides this, 1 year = 31536000)
  request->send(response);
  Serial.println(F("Serve Basic HTML Page: done"));
}
void toConfigPage(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", config_html, processorConfig);
  // response->addHeader("Cache-Control", "public,max-age=2000"); // cache the site this duration (refreshing overrides this, 1 year = 31536000)
  request->send(response);
}

void catchAll(AsyncWebServerRequest *request)
{
  Serial.printf("NOT_FOUND: ");
  if (request->method() == HTTP_GET)
    Serial.printf("GET");
  else if (request->method() == HTTP_POST)
    Serial.printf("POST");
  else if (request->method() == HTTP_DELETE)
    Serial.printf("DELETE");
  else if (request->method() == HTTP_PUT)
    Serial.printf("PUT");
  else if (request->method() == HTTP_PATCH)
    Serial.printf("PATCH");
  else if (request->method() == HTTP_HEAD)
    Serial.printf("HEAD");
  else if (request->method() == HTTP_OPTIONS)
    Serial.printf("OPTIONS");
  else
    Serial.printf("UNKNOWN");
  toPortal(request);
}

void addWebInterfaceHandlers(AsyncWebServer &server)
{
  server.on("/connecttest.txt", toLogout);    // windows 11 captive portal workaround
  server.on("/wpad.dat", toNotFound);         // stops win 10 calling this repeatedly
  server.on("/chat", toNotFound);             // pacify Whatsapp
  server.on("/favicon.ico", toNotFound);      // no favicon
  //server.on("/generate_204", toNotFound);     // no internet
  server.on("/success.txt", toOK);            // firefox captive portal call home
  server.on("/chrome-variations/seed", toOK); // chrome captive portal call home
  server.on("/service/update2/json", toOK);   // chrome updater
  server.on("/style.css", toCss);             // global css
  server.on("/config.html", toConfigPage);    // configuration page
  server.on("/", HTTP_ANY, toMainPage);
  server.onNotFound(toPortal); // redirect everthing else to the start page
}
