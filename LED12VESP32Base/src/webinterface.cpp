#include <webinterface.h>
#include <wifi_handler.h>

Stream *debug_uart_web_api = nullptr;

const char index_html[] = PROGMEM R"rawliteral(
<!doctypehtml><title>ESP32 LED Strip Captive Portal</title><meta content="width=device-width,initial-scale=1"name="viewport"><link href="data:,"rel="icon"><style>html{font-family:Arial;display:inline-block;text-align:center}h1{font-size:2rem;color:#fae1d2}h2{font-size:1.5rem;color:#fae1d2}p{font-size:1.2rem;color:#e1e1e1}body{max-width:600px;margin:0 auto;padding-bottom:25px;background-color:#133592}.switch{position:relative;display:inline-block;width:120px;height:68px}.switch input{display:none}.slider{position:absolute;top:0;left:0;right:0;bottom:0;background-color:#ccc;border-radius:6px}.slider:before{position:absolute;content:"";height:52px;width:52px;left:8px;bottom:8px;background-color:#fff;-webkit-transition:.4s;transition:.4s;border-radius:3px}input:checked+.slider{background-color:#b30000}input:checked+.slider:before{-webkit-transform:translateX(52px);-ms-transform:translateX(52px);transform:translateX(52px)}</style><h1>ESP32 LED Strip</h1><h2>Light is %LIGHT_STATE% (%LIGHT_BRIGHT%%)</h2><h2>Night Light is %NIGHT_LIGHT_STATE% (%NIGHT_LIGHT_BRIGHT%%)</h2><p>LDR: %LDR%<p>Presence (motion): %PRES_MOT%<p>Presence (stationary): %PRES_STAT%</p></body></html>
)rawliteral";

String localIPURL()
{
  WifiStateInfo wifiInfo = wifiCurrentState();
  return "http://" + wifiInfo.address.toString(); // a string version of the local IP with http, used for redirecting clients to your webpage
}

void webInterfaceDebug(Stream &terminalStream) { debug_uart_web_api = &terminalStream; }

// AwsTemplateProcessor
String processor(const String& var)
{
  if(var == "LIGHT_STATE")
  {

  }
  if(var == "LIGHT_BRIGHT")
  {

  }
  if(var == "NIGHT_LIGHT_STATE")
  {

  }
  if(var == "NIGHT_LIGHT_BRIGHT")
  {

  }
  if(var == "LDR")
  {

  }
  if(var == "PRES_MOT")
  {

  }
  if(var == "PRES_STAT")
  {
    
  }
  return String();
}

void webInterfaceSetup()
{
}

void webInterfaceLoop()
{
}

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
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, processor);
  response->addHeader("Cache-Control", "public,max-age=2000");  // save this file to cache for 1 year = 31536000 (unless you refresh)
  request->send(response);
  Serial.println(F("Serve Basic HTML Page: done"));
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

void addHandlers(AsyncWebServer &server)
{
  server.on("/connecttest.txt", toLogout);    // windows 11 captive portal workaround
  server.on("/wpad.dat", toNotFound);         // stops win 10 calling this repeatedly
  server.on("/chat", toNotFound);             // pacify Whatsapp
  server.on("/favicon.ico", toNotFound);      // no favicon
  server.on("/success.txt", toOK);            // firefox captive portal call home
  server.on("/chrome-variations/seed", toOK); // chrome captive portal call home
  server.on("/service/update2/json", toOK);   // chrome updater

  // all caught by server.onNotFound
  /*
  server.on("/generate_204", toPortal);        // android captive portal redirect
  server.on("/redirect", toPortal);            // microsoft redirect
  server.on("/hotspot-detect.html", toPortal); // apple call home
  server.on("/canonical.html", toPortal);      // firefox captive portal call home
  server.on("/ncsi.txt", toPortal);            // windows call home
  */

  server.on("/", HTTP_ANY, toMainPage);

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String inputMessage;
      String inputParam;
  
      if (request->hasParam("name")) {
        inputMessage = request->getParam("name")->value();
        inputParam = "name";
        //user_name = inputMessage;
        Serial.println(inputMessage);
        //name_received = true;
      }

      if (request->hasParam("proficiency")) {
        inputMessage = request->getParam("proficiency")->value();
        inputParam = "proficiency";
        //proficiency = inputMessage;
        Serial.println(inputMessage);
        //proficiency_received = true;
      }
      request->send(200, "text/html", "The values entered by you have been successfully sent to the device <br><a href=\"/\">Return to Home Page</a>"); });

  server.onNotFound(toPortal); // redirect everthing else to the start page
}
