#include <webinterface.h>
#include <wifi_handler.h>
#include <device_state.h>
#include <device_common.h>

Stream *debug_uart_web_api = nullptr;

// the css for all web pages, minified (-> https://www.toptal.com/developers/cssminifier)
const char style_css[] = PROGMEM R"rawliteral(
h1,h2,h3{color:#fae1d2}label,p{color:#e1e1e1}input,label{width:40%;display:inline-block}html,input,label{display:inline-block}html{font-family:Arial;text-align:left}h1{font-size:2rem}h2{font-size:1.5rem}h3{font-size:1.2rem}p{font-size:1rem}label{font-size:125%}input{font-size:1rem;color:#000}button{padding:.4em .8em;background:#08173f;border:thin solid #1c4eda;color:#fff;text-shadow:0 -.05em .05em #333;font-size:125%;line-height:1.5}body{max-width:600px;margin:0 auto;padding-bottom:25px;background-color:#133592}.group{background-color:#1640b4}.required{text-decoration:underline}
)rawliteral";

// the inital web site (see index.html), minified (-> https://htmlminifier.com/)
const char index_html[] = PROGMEM R"rawliteral(
<!doctypehtml><title>ESP32 LED Strip</title><meta content="width=device-width,initial-scale=1"name="viewport"><link href="style.css"rel="stylesheet"><link href="data:,"rel="icon"><body><script>function configButton(){setTimeout(function(){window.open("config.html","_self")},300)}</script><h1>ESP32 LED Strip</h1><p>Firmware: %FWM%.%FWI%.%FWP%<h2>Light is %LI1% ( %LIB% )</h2><h2>Night Light is %NL1% ( %NLB% )</h2><p>LDR: %LDR%, thres %LDT%<p>Motion: %PM%<p>distance: %PM1% , min %PM2%, max %PM3%<p>energy: %PM4% , min %PM5%, max %PM6%<p>Presence (stationary): %PS%<p>distance: %PS1% , min %PS2%, max %PS3%<p>energy: %PS4% , min %PS5%, max %PS6%<p>No Presence duration: %PND% , max %PNM%</p><button onclick="configButton()">Configuration</button>
)rawliteral";

// the configuration web site (see config.html), minified (-> https://htmlminifier.com/)
const char config_html[] = PROGMEM R"rawliteral(
<!doctypehtml><html lang="en"><title>ESP32 LED Strip Configuration</title><meta content="width=device-width,initial-scale=1"name="viewport"><link href="style_gen.css"rel="stylesheet"><link href="data:,"rel="icon"><body><script>function backButton(){setTimeout(function(){window.open("index.html","_self")},300)}</script><h1>Configuration</h1><p>Mandatory values are underlined.<div class="category"><h2>Light</h2><div class="group"><p>Lower values mean lower brightness. Allowed values: 1..255.<div title="default: 210"><form action="/v1/post"method="post"><label for="mbr">Max brighteness in light mode:</label> <input id="mbr"name="mbr"type="number"inputmode="decimal"max="255"min="1"step="1"value="210"> <button name="bmbr"value="1">Set</button></form></div><div title="default: 210"><form action="/v1/post"method="post"><label for="obr">Brighteness in light mode:</label> <input id="obr"name="obr"type="number"inputmode="decimal"max="255"min="1"step="1"value="210"> <button name="bobr"value="1">Set</button></form></div></div></div><div class="category"><h2>Nightlight</h2><div class="group"><div title="default: True"><form action="/v1/post"method="post"><label for="alnl">Allow nightlight mode:</label> <input id="alnl"name="alnl"type="checkbox"> <button name="balnl"value="1">Set</button></form></div></div><div class="group"><p>Lower values mean lower brightness. Allowed values: 1..128.<div title="default: 128"><form action="/v1/post"method="post"><label for="nlbr">Max brighteness in nightlight mode:</label> <input id="nlbr"name="nlbr"type="number"inputmode="decimal"max="128"min="1"step="1"value="128"> <button name="bnlbr"value="1">Set</button></form></div><div title="default: 8"><form action="/v1/post"method="post"><label for="mnlb">Brighteness in nightlight mode:</label> <input id="mnlb"name="mnlb"type="number"inputmode="decimal"max="128"min="1"step="1"value="8"> <button name="bmnlb"value="1">Set</button></form></div></div><div class="group"><p>Allowed values: 1..600.<div title="default: 30"><form action="/v1/post"method="post"><label for="odu">On duration (seconds):</label> <input id="odu"name="odu"type="number"inputmode="decimal"max="600"min="1"step="1"value="30"> <button name="bodu"value="1">Set</button></form></div></div><div class="group"><p>Brightness detection, lower values mean lower brightness. Allowed values: 1..4095.<div title="default: 30"><form action="/v1/post"method="post"><label for="nllt">LDR Threshold:</label> <input id="nllt"name="nllt"type="number"inputmode="decimal"max="4095"min="1"step="1"value="30"> <button name="bnllt"value="1">Set</button></form></div></div></div><div class="category"><h2>Presence detection</h2><div class="group"><h3>Distance</h3><p>Distance is a generic value (not meters or the like). Allowed values: 30..800.<div title="default: 800"><form action="/v1/post"method="post"><label for="mamd">Max moving target distance:</label> <input id="mamd"name="mamd"type="number"inputmode="decimal"max="800"min="30"step="1"value="800"> <button name="bmamd"value="1">Set</button></form></div><div title="default: 30"><form action="/v1/post"method="post"><label for="mimd">Min moving target distance:</label> <input id="mimd"name="mimd"type="number"inputmode="decimal"max="800"min="30"step="1"value="30"> <button name="bmimd"value="1">Set</button></form></div><div title="default: 800"><form action="/v1/post"method="post"><label for="masd">Max stationary target distance:</label> <input id="masd"name="masd"type="number"inputmode="decimal"max="800"min="30"step="1"value="800"> <button name="bmasd"value="1">Set</button></form></div><div title="default: 30"><form action="/v1/post"method="post"><label for="misd">Min stationary target distance:</label> <input id="misd"name="misd"type="number"inputmode="decimal"max="800"min="30"step="1"value="30"> <button name="bmisd"value="1">Set</button></form></div></div><div class="group"><h3>Energy</h3><p>Read "energy" as "certainty". Allowed values: 0..100.<div title="default: 100"><form action="/v1/post"method="post"><label for="mame">Max moving target energy:</label> <input id="mame"name="mame"type="number"inputmode="decimal"max="100"min="0"step="1"value="100"> <button name="bmame"value="1">Set</button></form></div><div title="default: 0"><form action="/v1/post"method="post"><label for="mime">Min moving target energy:</label> <input id="mime"name="mime"type="number"inputmode="decimal"max="100"min="0"step="1"value="0"> <button name="bmime"value="1">Set</button></form></div><div title="default: 100"><form action="/v1/post"method="post"><label for="mase">Max stationary target energy:</label> <input id="mase"name="mase"type="number"inputmode="decimal"max="100"min="0"step="1"value="100"> <button name="bmase"value="1">Set</button></form></div><div title="default: 0"><form action="/v1/post"method="post"><label for="mise">Min stationary target energy:</label> <input id="mise"name="mise"type="number"inputmode="decimal"max="100"min="0"step="1"value="0"> <button name="bmise"value="1">Set</button></form></div></div></div><div class="category"><h2>Network</h2><div class="group"><h3>Web interface login</h3><p>When password is empty, the web interface will be accessible without a login (NOT recommended!).<div title="default: admin"><form action="/v1/post"method="post"><label for="waun"class="required">User:</label> <input id="waun"name="waun"required value="admin"> <button name="bwaun"value="1">Set</button></form></div><div title="default: lamp"><form action="/v1/post"method="post"><label for="wapw">Password:</label> <input id="wapw"name="wapw"type="password"autocomplete="off"spellcheck="false"> <button name="bwapw"value="1">Set</button></form></div></div><div class="group"><h3>WiFi Access</h3><p>When SSID is empty, the lamp will not try to connect to a WiFi network. The lamp will boot into Access Point Mode when the credentials are invalid.<div title="default: "><form action="/v1/post"method="post"><label for="wsss">WiFi network name (SSID):</label> <input id="wsss"name="wsss"> <button name="bwsss"value="1">Set</button></form></div><div title="default: "><form action="/v1/post"method="post"><label for="wspa">Password:</label> <input id="wspa"name="wspa"type="password"autocomplete="off"spellcheck="false"> <button name="bwspa"value="1">Set</button></form></div><div title="default: esp32LEDStrip"><form action="/v1/post"method="post"><label for="whon"class="required">Hostname (max len 32):</label> <input id="whon"name="whon"required value="esp32LEDStrip"> <button name="bwhon"value="1">Set</button></form></div></div><div class="group"><h3>Access Point</h3><p>To access the access point without a password, leave Password empty.<div title="default: esp32LEDStrip"><form action="/v1/post"method="post"><label for="wass"class="required">Access Point network name (SSID):</label> <input id="wass"name="wass"required value="esp32LEDStrip"> <button name="bwass"value="1">Set</button></form></div><div title="default: "><form action="/v1/post"method="post"><label for="wapa">Password:</label> <input id="wapa"name="wapa"type="password"autocomplete="off"spellcheck="false"> <button name="bwapa"value="1">Set</button></form></div><div title="default: 192.168.72.12"><form action="/v1/post"method="post"><label for="waip"class="required">IPv4 address:</label> <input id="waip"name="waip"required minlength="7"maxlength="15"size="15"pattern="^((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$"value="192.168.72.12"> <button name="bwaip"value="1">Set</button></form></div></div><div class="group"><h3>Access Point (TEST)</h3><div title="default: 255.255.255.0"><form action="/v1/post"method="post"><label for="wanm"class="required">IPv4 net mask:</label> <input id="wanm"name="wanm"required minlength="7"maxlength="15"size="15"pattern="^((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$"value="255.255.255.0"> <button name="bwanm"value="1">Set</button></form></div></div><div class="group"><h3>MQTT</h3><div title="default: "><form action="/v1/post"method="post"><label for="mqsv">Server address:</label> <input id="mqsv"name="mqsv"> <button name="bmqsv"value="1">Set</button></form></div><div title="default: "><form action="/v1/post"method="post"><label for="mqus">Username:</label> <input id="mqus"name="mqus"> <button name="bmqus"value="1">Set</button></form></div><div title="default: "><form action="/v1/post"method="post"><label for="mqpw">Password:</label> <input id="mqpw"name="mqpw"type="password"autocomplete="off"spellcheck="false"> <button name="bmqpw"value="1">Set</button></form></div></div></div><div class="category"><h2>System</h2><div class="group"><div title="default: 1000"><form action="/v1/post"method="post"><label for="ptdm">Brightness transition duration (millisecs):</label> <input id="ptdm"name="ptdm"type="number"inputmode="decimal"max="10000"min="1"step="1"value="1000"> <button name="bptdm"value="1">Set</button></form></div></div></div><button onclick="backButton()">Back</button>
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

  if (var == "FWM")
  {
    return String(FIRMWARE_VERSION_MAJOR);
  }
  if (var == "FWI")
  {
    return String(FIRMWARE_VERSION_MINOR);
  }
  if (var == "FWP")
  {
    return String(FIRMWARE_VERSION_PATCH);
  }
  if (var == "LI1")
  {
    return isOn ? "ON" : "OFF";
  }
  if (var == "LIB")
  {
    return isOn ? String(info.onBrightness) : "0";
  }
  if (var == "NL1")
  {
    return String(isNightLightOn ? "ON" : "OFF") + String(isNightLightOn ? "" : String(info.allowNightLightMode ? " (enabled)" : " (disabled)"));
  }
  if (var == "NLB")
  {
    return isNightLightOn ? String(info.nightLightBrightness) : "0";
  }
  if (var == "LDR")
  {
    return String(info.ldrValue);
  }
  if (var == "LDT")
  {
    return String(info.nightLightThreshold);
  }
  if (var == "PM")
  {
    return info.movingTargetDetected ? "YES" : "NO";
  }
  if (var == "PM1")
  {
    return String(info.movingTargetDistance);
  }
  if (var == "PM2")
  {
    return String(info.movingTargetDistanceMin);
  }
  if (var == "PM3")
  {
    return String(info.movingTargetDistanceMax);
  }
  if (var == "PM4")
  {
    return String(info.movingTargetEnergy);
  }
  if (var == "PM5")
  {
    return String(info.movingTargetEnergyMin);
  }
  if (var == "PM6")
  {
    return String(info.movingTargetEnergyMax);
  }
  if (var == "PS")
  {
    return info.stationaryTargetDetected ? "YES" : "NO";
  }
  if (var == "PS1")
  {
    return String(info.stationaryTargetDistance);
  }
  if (var == "PS2")
  {
    return String(info.stationaryTargetDistanceMin);
  }
  if (var == "PS3")
  {
    return String(info.stationaryTargetDistanceMax);
  }
  if (var == "PS4")
  {
    return String(info.stationaryTargetEnergy);
  }
  if (var == "PS5")
  {
    return String(info.stationaryTargetEnergyMin);
  }
  if (var == "PS6")
  {
    return String(info.stationaryTargetEnergyMax);
  }
  if (var == "PND")
  {
    return String(info.noPresenceDuration);
  }
  if (var == "PNM")
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
  Serial.printf(F("NOT_FOUND: "));
  if (request->method() == HTTP_GET)
    Serial.printf(F("GET"));
  else if (request->method() == HTTP_POST)
    Serial.printf(F("POST"));
  else if (request->method() == HTTP_DELETE)
    Serial.printf(F("DELETE"));
  else if (request->method() == HTTP_PUT)
    Serial.printf(F("PUT"));
  else if (request->method() == HTTP_PATCH)
    Serial.printf(F("PATCH"));
  else if (request->method() == HTTP_HEAD)
    Serial.printf(F("HEAD"));
  else if (request->method() == HTTP_OPTIONS)
    Serial.printf(F("OPTIONS"));
  else
    Serial.printf(F("UNKNOWN"));
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
