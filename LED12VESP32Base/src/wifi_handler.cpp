/*

Learned lots from https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/

 */

#include <WiFi.h>

#include "wifi_handler.h"
#include "config.h"
#include <DNSServer.h>

WifiState _wifiState = NO_WIFI_YET; // The state the wifi finite state machine is currently in.
unsigned long _wifiStateTs = 0;     // The moment in time when the finite state machine entered the current _state.
WifiMode _wifiMode = WifiMode::WifiMode_OFF;

char _hostname[MAX_HOSTNAME_LEN] = {0};

char _ap_ssid[MAX_SSID_LEN] = {0};                     // The currently used SSID of the AP
char _ap_passphrase[MAX_PASSPHRASE_LEN] = {0};         // The currently used passphrase for connecting to the AP
uint32_t _ap_ip = 0;                                   // The currently used IP address of the AP
uint32_t _ap_netmask = 0;                              // The currently used netmask of the AP
char _ap_ssid_working[MAX_SSID_LEN] = {0};             // For reverting bad changes to the AP configuration
char _ap_passphrase_working[MAX_PASSPHRASE_LEN] = {0}; // For reverting bad changes to the AP configuration
uint32_t _ap_ip_working = 0;                           // For reverting bad changes to the AP configuration
uint32_t _ap_netmask_working = 0;                      // For reverting bad changes to the AP configuration
bool _apTestChangedSettings = false;                   // Whether we have a changed AP configuration that is not tested yet. Set to true, when ssid, ip or netmask change!

char _sta_ssid[MAX_SSID_LEN] = {0};             // STA will try to connect to the WiFi with this ssid
char _sta_passphrase[MAX_PASSPHRASE_LEN] = {0}; // STA will try to connect to the WiFi with this passphrase

IPAddress _ipAddress = IPAddress((uint32_t)0);

Stream *debug_uart_wifi = nullptr;

ModeResult _apModeResult = ModeResult::MODE_NOT_ATTEMPTED_YET; // Result of starting STA mode with the current ssid and passphrase. Reset to NOT_ATTEMPTED_YET when ssid, ip or netmask changes!

// info about STA
wifi_err_reason_t _staDisconnectReason = WIFI_REASON_UNSPECIFIED;

bool _staConnectedAtLeastOnce = false;                          // (1) Was there at least one successful attempt to connect as STA with the current ssid and passphrase?
bool _staConnected = false;                                     // (2) Is the device currently connected to a WiFi as STA?
bool _staFailed = false;                                        // (3) Has an initial attempt to connect as STA with the current ssid and passphrase failed?
ModeResult _staModeResult = ModeResult::MODE_NOT_ATTEMPTED_YET; // Result of starting STA mode with the current ssid and passphrase. Reset to NOT_ATTEMPTED_YET when ssid, ip or netmask change!
/*
  connection as STA     |  (1)  |  (2)  |  (3)  |
  not in STA mode       | false | false | false |
  not attempted yet     | false | false | false |
  success, connected    | true  | true  | false |
  success, disconnected | true  | false | false |
  failed, disconnected  | false | false | true  |
*/

// info about AP
bool _forceAPMode = false; // Requested by user: start in AP mode although there is a configuration for STA that worked at least once.

DNSServer dnsServer; // for providing a captive portal in AP mode

WifiStateInfo wifiCurrentState()
{
  WifiStateInfo info;
  info.currentState = _wifiState;
  info.enteredStateAtTs = _wifiStateTs;
  info.mode = _wifiMode;
  info.apModeResult = _apModeResult;
  info.staModeResult = _staModeResult;
  info.address = _ipAddress;
  return info;
}

size_t debugPrint(const __FlashStringHelper *ifsh) { return debug_uart_wifi != nullptr ? debug_uart_wifi->print(ifsh) : 0; }
size_t debugPrint(const char *text) { return debug_uart_wifi != nullptr ? debug_uart_wifi->print(text) : 0; }
size_t debugPrint(const unsigned char value, const int base = DEC) { return debug_uart_wifi != nullptr ? debug_uart_wifi->print(value, base) : 0; }

size_t debugPrintln(const __FlashStringHelper *ifsh) { return debug_uart_wifi != nullptr ? debug_uart_wifi->println(ifsh) : 0; }
size_t debugPrintln(const char *text) { return debug_uart_wifi != nullptr ? debug_uart_wifi->println(text) : 0; }
size_t debugPrintln(const unsigned char value, const int base = DEC) { return debug_uart_wifi != nullptr ? debug_uart_wifi->println(value, base) : 0; }
size_t debugPrintln(const Printable &value) { return debug_uart_wifi != nullptr ? debug_uart_wifi->println(value) : 0; }
size_t debugPrintln() { return debug_uart_wifi != nullptr ? debug_uart_wifi->println() : 0; }

void wifiDebug(Stream &terminalStream)
{
  debug_uart_wifi = &terminalStream;
}

void debugPrintMAC(const uint8_t mac[6], bool addPrintln = false)
{
  for (int i = 5; i > 0; i--)
  {
    Serial.print(mac[i], HEX);
    Serial.print(F(":"));
  }
  if (addPrintln)
  {
    Serial.println();
  }
}

void debugPrintModeResult(Stream *stream, ModeResult result, bool addPrintln)
{
  if (stream != nullptr)
  {
    switch (result)
    {
    case ModeResult::MODE_FAIL:
      stream->print(F("MODE_FAIL"));
      break;

    case ModeResult::MODE_NOT_ATTEMPTED_YET:
      stream->print(F("MODE_NOT_ATTEMPTED_YET"));
      break;

    case ModeResult::MODE_SUCCESS:
      stream->print(F("MODE_SUCCESS"));
      break;

    default:
      stream->print(F("UNKNOWN ModeResult - THIS IS A BUG"));
      break;
    }
    if (addPrintln)
      debug_uart_wifi->println();
  }
}

void debugPrintModeResult(ModeResult result, bool addPrintln = false) { debugPrintModeResult(debug_uart_wifi, result, addPrintln); }

void debugPrintWifiMode(Stream *stream, WifiMode mode, bool addPrintln)
{
  if (stream != nullptr)
  {
    switch (mode)
    {
    case WifiMode::WifiMode_AP:
      stream->print(F("WifiMode_AP"));
      break;
    case WifiMode::WifiMode_OFF:
      stream->print(F("WifiMode_OFF"));
      break;
    case WifiMode::WifiMode_STA:
      stream->print(F("WifiMode_STA"));
      break;
    default:
      stream->print(F("UNKNOWN WifiMode - THIS IS A BUG"));
      break;
    }
    if (addPrintln)
      debug_uart_wifi->println();
  }
}

void debugPrintWifiMode(WifiMode mode, bool addPrintln = false) { debugPrintWifiMode(debug_uart_wifi, mode, addPrintln); }

void debugPrintWifiState(Stream *stream, WifiState state, bool addPrintln)
{
  if (stream != nullptr)
  {
    switch (state)
    {
    /*case UNHINGE:
      debug_uart_wifi->print(F("UNHINGE"));
      break;*/
    case NO_WIFI_YET:
      stream->print(F("NO_WIFI_YET"));
      break;
    case START_STA_OR_AP:
      stream->print(F("START_STA_OR_AP"));
      break;
    case STA_START:
      stream->print(F("STA_START"));
      break;
    case STA_START_WAIT:
      stream->print(F("STA_START_WAIT"));
      break;
    case STA_OK:
      stream->print(F("STA_OK"));
      break;
    case STA_FAIL:
      stream->print(F("STA_FAIL"));
      break;
    case STA_LOST_CONNECTION:
      stream->print(F("STA_LOST_CONNECTION"));
      break;
    case STA_RECONNECT:
      stream->print(F("STA_RECONNECT"));
      break;
    case STA_RECONNECT_WAIT:
      stream->print(F("STA_RECONNECT_WAIT"));
      break;
    case STA_SWITCH_TO_AP:
      stream->print(F("STA_SWITCH_TO_AP"));
      break;
    case STA_SWITCH_WAIT_STA_DOWN:
      stream->print(F("STA_SWITCH_WAIT_STA_DOWN"));
      break;
    case AP_START:
      stream->print(F("AP_START"));
      break;
    case AP_START_WAIT:
      stream->print(F("AP_START_WAIT"));
      break;
    case AP_OK:
      stream->print(F("AP_OK"));
      break;
    case AP_SWITCH_TO_STA:
      stream->print(F("AP_SWITCH_TO_STA"));
      break;
    case AP_SWITCH_WAIT_AP_DOWN:
      stream->print(F("AP_SWITCH_WAIT_AP_DOWN"));
      break;
    case AP_FAIL:
      stream->print(F("AP_FAIL"));
      break;
    default:
      stream->print(F("UNKNOWN WifiState - THIS IS A BUG"));
      break;
    }
    if (addPrintln)
      debug_uart_wifi->println();
  }
}

void debugPrintWifiState(WifiState state, bool addPrintln = false) { debugPrintWifiState(debug_uart_wifi, state, addPrintln); }

void debugPrintEvent() { debugPrint(F("[Event] ")); }

void printWifi() { Serial.print(F("[Wifi] ")); }

// Set a new state for the state machine
void setState(WifiState newState)
{
  if (_wifiState != newState)
  {
    _wifiStateTs = millis();
    if (debug_uart_wifi != nullptr)
    {
      debug_uart_wifi->print(F("[WiFi] => change state from "));
      debugPrintWifiState(_wifiState);
      debug_uart_wifi->print(F(" to "));
      debugPrintWifiState(newState, true);
    }
  }
  _wifiState = newState;
}

void requestAPMode()
{
  _forceAPMode = true;
}

// ARDUINO_EVENT_WIFI_READY
void wifiEventReady(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("WiFi interface ready"));
  // ARDUINO_EVENT_WIFI_READY has no WiFiEventInfo_t info
}

// ARDUINO_EVENT_WIFI_SCAN_DONE
void wifiEventScanDone(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrint(F("Completed scan for access points (found "));
  debugPrint(info.wifi_scan_done.number);
  debugPrintln(F(")"));
}

// ARDUINO_EVENT_WIFI_STA_START
void wifiEventStaStart(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("WiFi client started"));
  // ARDUINO_EVENT_WIFI_STA_START has no WiFiEventInfo_t info
}

// ARDUINO_EVENT_WIFI_STA_STOP
void wifiEventStaStop(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("WiFi client stopped"));
  // ARDUINO_EVENT_WIFI_STA_STOP has no WiFiEventInfo_t info
}

// ARDUINO_EVENT_WIFI_STA_CONNECTED
void wifiEventStaConnected(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  Serial.print(F("Connected to access point, ssid_len: "));
  debugPrintln(info.wifi_sta_connected.ssid_len);
}

// ARDUINO_EVENT_WIFI_STA_DISCONNECTED
void wifiEventStaDisconnected(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrint(F("Disconnected from WiFi access point, reason:"));
  // info.wifi_sta_disconnected.reason is a uint8_t, wifi_err_reason_t is an enum with a max number of 209 -> conversion should be save
  wifi_err_reason_t reason_t = (wifi_err_reason_t)info.wifi_sta_disconnected.reason;
  debugPrintln(WiFi.disconnectReasonName(reason_t));
}

// ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE
void wifiEventStaAuthmodeChanged(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("Authentication mode of access point has changed, new mode: "));
  debugPrintln(info.wifi_sta_authmode_change.new_mode);
}

// ARDUINO_EVENT_WIFI_STA_GOT_IP
void wifiEventStaGotIP(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("WiFi connected, IPv4 address: "));
  debugPrintln(IPAddress(info.got_ip.ip_info.ip.addr));
}

// ARDUINO_EVENT_WIFI_STA_GOT_IP6
void wifiEventStaGotIP6(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("WiFi connected, IPv6 address: "));
  debugPrintln(IPv6Address(info.got_ip6.ip6_info.ip.addr));
}

// ARDUINO_EVENT_WIFI_STA_LOST_IP
void wifiEventStaLostIP(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("Lost IP address, IP address is reset to 0"));
  // ARDUINO_EVENT_WIFI_STA_LOST_IP has no WiFiEventInfo_t info
}

// ARDUINO_EVENT_WIFI_AP_START
void wifiEventApStart(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("WiFi access point started"));
  // ARDUINO_EVENT_WIFI_AP_START has no WiFiEventInfo_t info
  _apModeResult = ModeResult::MODE_SUCCESS;
}

// ARDUINO_EVENT_WIFI_AP_STOP
void wifiEventApStop(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrintln(F("WiFi access point stopped"));
  // ARDUINO_EVENT_WIFI_AP_STOP has no WiFiEventInfo_t info
}

// ARDUINO_EVENT_WIFI_AP_STACONNECTED
void wifiEventApStaConnected(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrint(F("AP: Client connected, MAC: "));
  debugPrintMAC(info.wifi_ap_staconnected.mac, true);
}

// ARDUINO_EVENT_WIFI_AP_STADISCONNECTED
void wifiEventApStaDisconnected(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrint(F("AP: Client disconnected, MAC: "));
  debugPrintMAC(info.wifi_ap_stadisconnected.mac, true);
}

// ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED
void wifiEventApStaIpAssigned(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrint(F("AP: Assigned IPv4 address to client: "));
  debugPrintln(IPAddress(info.wifi_ap_staipassigned.ip.addr));
}

// ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED
void wifiEventApProbeReqRecved(const WiFiEvent_t &event, const WiFiEventInfo_t &info)
{
  debugPrintEvent();
  debugPrint(F("AP: Received probe request, MAC: "));
  debugPrintMAC(info.wifi_ap_probereqrecved.mac, true);
}

// Initialize Wifi to off
WifiState handleNoWifiYet()
{
  WiFi.disconnect(true, false); // also turn WiFi radio off but don't erase the info about the AP to connect to as STA
  WiFi.setHostname(_hostname);
  _wifiMode = WifiMode::WifiMode_OFF;
  _ipAddress = IPAddress((uint32_t)0);
  return START_STA_OR_AP;
}

WifiState handleStartStaOrAp()
{
  printWifi();
  Serial.println(F("Deciding whether to start AP or to connect to a WiFi as STAtion"));

  bool staFailed = _staModeResult == ModeResult::MODE_FAIL;
  bool staSsidNotConfigured = (0 == strlen(_sta_ssid));
  if (_forceAPMode || staFailed || staSsidNotConfigured)
  {
    if (_forceAPMode)
    {
      printWifi();
      Serial.println(F("User has requested AP mode"));
      _forceAPMode = false; // do not force AP again on next mode switch
    }
    else if (staFailed)
    {
      printWifi();
      Serial.println(F("Previous attempt to connect to WiFi has failed"));
    }
    else
    {
      printWifi();
      Serial.println(F("Configuration lacks a WiFi SSID to connect to"));
    }

    // Try to start in AP mode when it has either not been started with the current configuration before or the last attempt was successful.
    switch (_apModeResult)
    {
    case ModeResult::MODE_NOT_ATTEMPTED_YET:
      printWifi();
      Serial.print(F("Starting AP "));
      Serial.println(_apTestChangedSettings ? F("with changed setting") : F("for the first time since power on"));
      return AP_START;
    case ModeResult::MODE_SUCCESS:
      printWifi();
      Serial.print(F("Starting AP "));
      Serial.println(F("(again)"));
      return AP_START;
    case ModeResult::MODE_FAIL:
      break;
    }

    // The previous attempt to start in AP mode has failed, possibly due to a bad configuration change -> revert to the (hopefully) working previous configuration.
    printWifi();
    Serial.print(F("Previous attempt to start AP mode has failed"));
    if (_ap_ip != _ap_ip_working || _ap_netmask != _ap_netmask_working || /*_ap_passphrase != _ap_passphrase_working || */ _ap_ssid != _ap_ssid_working)
    {
      Serial.println(F(" restoring working AP configuration"));
      _ap_ip = _ap_ip_working;
      _ap_netmask = _ap_netmask_working;
      //_ap_passphrase = _ap_passphrase_working;
      strncpy(_ap_ssid, _ap_ssid_working, sizeof(_ap_ssid));
      // Try again in next loop cycle
      _apModeResult = ModeResult::MODE_NOT_ATTEMPTED_YET;
      return AP_START;
    }

    Serial.println(F(", no working AP configuration found. This is fatal, neither AP nor STA work! Disabling WiFi altogether."));
    return NO_WIFI_PERM;
  }
  // Either no attempt to start in STA mode has been made yet or the last attempt was successful -> (try to) start STA mode
  printWifi();
  Serial.print(F("Try to connect to "));
  Serial.println(_staModeResult == ModeResult::MODE_SUCCESS ? F("previously used WiFi.") : F("configured WiFi."));
  return STA_START;
}

WifiState handleStaStart()
{
  printWifi();
  Serial.print(F("Started connecting to WiFi "));
  Serial.print(_sta_ssid);
  Serial.print(F(" and "));
  if (0 == strlen(_sta_passphrase))
    Serial.print(F("no "));
  Serial.println(F("password."));

  _wifiMode = WifiMode::WifiMode_STA;
  if (!WiFi.mode(WIFI_STA))
  {
    printWifi();
    Serial.println(F("Connecting to WiFi failed: could not start WIFI_STA mode."));
    return STA_FAIL;
  }
  if (!WiFi.begin(_sta_ssid, _sta_passphrase))
  {
    printWifi();
    Serial.println(F("Connecting to WiFi failed: WiFi.begin failed"));
    return STA_FAIL;
  }

  WiFi.setAutoReconnect(true);
  return STA_START_WAIT;
}

WifiState handleStaStartWait()
{
  switch (WiFi.status())
  {
  case WL_NO_SSID_AVAIL:
    printWifi();
    Serial.println(F("SSID not found"));
    break;

  case WL_CONNECT_FAILED:
    printWifi();
    Serial.print(F("Connect failed"));
    break;

  case WL_CONNECTION_LOST:
    printWifi();
    Serial.println(F("Connection lost"));
    break;

  case WL_SCAN_COMPLETED:
    printWifi();
    Serial.println(F("Scan completed"));
    return STA_START_WAIT; // still waiting for WL_CONNECTED
    break;

  case WL_DISCONNECTED:
    printWifi();
    Serial.println(F("is disconnected"));
    break;

  case WL_CONNECTED:
    printWifi();
    Serial.print(F("is connected, IP address: "));
    _ipAddress = WiFi.localIP();
    Serial.println(_ipAddress);
    return STA_OK; // success
    break;

  default:
    printWifi();
    Serial.print(F("Status: "));
    Serial.println(WiFi.status());
    return STA_START_WAIT; // still waiting for WL_CONNECTED
    break;
  }
  return STA_FAIL;
}

WifiState handleStaOk()
{
  // stay in this state (ARDUINO_EVENTs may change the state though)
  return STA_OK;
}

WifiState handleStaFail()
{
  printWifi();
  Serial.println(F("STA failed"));
  _staModeResult == ModeResult::MODE_FAIL;
  return NO_WIFI_YET;
}

WifiState handleStaLostConnection()
{
  printWifi();
  Serial.println(F("Connection lost"));
  return STA_RECONNECT;
}

WifiState handleStaReconnect()
{
  bool autoReconnect = WiFi.getAutoReconnect();
  printWifi();
  Serial.println(autoReconnect ? F("waiting for automatic reconnection") : F("reconnecting"));
  if (!autoReconnect)
  {
    WiFi.reconnect();
  }
  return STA_RECONNECT_WAIT;
}

WifiState handleStaReconnectWait()
{
  bool connectionReestablished = WiFi.status() == WL_CONNECTED;
  if (connectionReestablished)
  {
    printWifi();
    Serial.println(F("Connection reestablished"));
  }
  return connectionReestablished ? STA_OK : STA_RECONNECT_WAIT;
}

WifiState handleStaSwitchToAp()
{
  printWifi();
  Serial.println(F("Switching from STA to AP, disconnecting from WiFi"));
  // Disconnect from WiFi, ignore return value because false means "there was no connection to a WiFi in the first place", which is the desired state anyway.
  // WiFi.disconnect returns false in these cases:
  // * ESP_ERR_WIFI_NOT_INIT: WiFi was not initialized by esp_wifi_init
  // * ESP_ERR_WIFI_NOT_STARTED: WiFi was not started by esp_wifi_start
  // * ESP_FAIL: other WiFi internal errors
  WiFi.disconnect(false, false); // do not switch off WiFi radio
  return STA_SWITCH_WAIT_STA_DOWN;
}

WifiState handleStaSwitchWaitStaDown()
{
  bool staIsDown = WiFi.isConnected();
  if (staIsDown)
  {
    printWifi();
    Serial.println(F("Switching from STA to AP, disconnected from WiFi"));
  }
  return staIsDown ? AP_START : STA_SWITCH_WAIT_STA_DOWN;
}

WifiState handleApStart()
{
  printWifi();
  Serial.print(F("Starting the AP, ssid: '"));
  Serial.print(_ap_ssid);
  Serial.print(F("', passphrase: '"));
  Serial.print(_ap_passphrase);
  Serial.println(F("'"));

  _wifiMode = WifiMode::WifiMode_AP;
  bool modeResult = WiFi.mode(WIFI_AP);
  printWifi();
  if (modeResult)
  {
    bool startResult;
    if (strlen(_ap_passphrase) == 0)
    {
      startResult = WiFi.softAP(_ap_ssid);
    }
    else
    {
      startResult = WiFi.softAP(_ap_ssid, _ap_passphrase);
    }
    if (startResult)
    {
      printWifi();
      Serial.println(F("SoftAP starting"));
      if (dnsServer.start(53, "*", WiFi.softAPIP()))
      {
        printWifi();
        Serial.println(F("DNS started"));
      }
      else
      {
        printWifi();
        Serial.println(F("DNS NOT started!"));
      }
      return AP_START_WAIT;
    }
    Serial.println(F("Could not start softAP."));
  }
  else
  {
    Serial.println(F("Could not set WIFI_AP mode."));
  }
  return AP_FAIL;
}

WifiState handleApStartWait()
{
  // wait until the AP is started, only then it can be configured successfully
  if (_apModeResult == ModeResult::MODE_SUCCESS)
  {
    IPAddress apIp = IPAddress(_ap_ip);
    IPAddress apNetmask = IPAddress(_ap_netmask);
    printWifi();
    Serial.println(F("Configuring the AP:"));
    Serial.print(F(" IP:      "));
    Serial.println(apIp);
    Serial.print(F(" Gateway: "));
    Serial.println(apIp);
    Serial.print(F(" Netmask: "));
    Serial.println(apNetmask);
    // Set AP configuration, use same IPAddress for local_ip and gateway.
    // This will trigger another ARDUINO_EVENT_WIFI_AP_START.
    printWifi();
    if (WiFi.softAPConfig(wifiApIPv4Address(), wifiApIPv4Address(), wifiApIPv4Netmask()))
    {
      Serial.println(F("AP configuration done"));
      if (_apTestChangedSettings)
      {
        Serial.println(F(" Saving new and working changed AP settings as default"));
        _ap_ip_working = _ap_ip;
        if (wifiApIPv4Address() != _ap_ip)
        {
          setWifiAPpIPv4Address(_ap_ip);
        }
        _ap_netmask_working = _ap_netmask;
        if (wifiApIPv4Netmask() != _ap_netmask)
        {
          setWifiAPpIPv4Netmask(_ap_netmask);
        }
        strncpy(_ap_ssid_working, _ap_ssid, sizeof(_ap_ssid_working));
        char savedSsid[MAX_SSID_LEN];
        getWifiApSsid(savedSsid, sizeof(savedSsid));
        if (strncmp(_ap_ssid, savedSsid, sizeof(savedSsid)) != 0)
        {
          setWifiApSsid(_ap_ssid, sizeof(_ap_ssid_working));
        }
        _apTestChangedSettings = false;
      }
      printWifi();
      Serial.print(F("AP is now available, IP: "));
      _ipAddress = WiFi.softAPIP();
      Serial.println(_ipAddress);
      return AP_OK;
    }
    Serial.println(F("Could not set AP configuration"));
    _apTestChangedSettings = false;
    return AP_FAIL;
  }
  return AP_START_WAIT;
}

WifiState handleApOk()
{
  // stay in this state (events may change the state though)
  return AP_OK;
}

WifiState handleApSwitchToSta()
{
  printWifi();
  Serial.println(F("Switching from AP to STA, stopping AP"));
  WiFi.enableAP(false);
  return AP_SWITCH_WAIT_AP_DOWN;
}

WifiState handleApSwitchWaitApDown()
{
  // found nothing to check here...
  printWifi();
  Serial.println(F("Switching from STA to AP, AP is stopped"));
  return STA_START;
}

WifiState handleApFail()
{
  printWifi();
  Serial.println(F("AP failed."));
  _apModeResult == ModeResult::MODE_FAIL;
  return NO_WIFI_YET;
}

/*






*/

void wifiSetup()
{
  _wifiStateTs = millis();

  getWifiApSsid(_ap_ssid, sizeof(_ap_ssid));
  strncpy(_ap_ssid_working, _ap_ssid, sizeof(_ap_ssid_working));
  getWifiStaPassphrase(_ap_passphrase, sizeof(_ap_passphrase));
  getWifiHostname(_hostname, sizeof(_hostname));
  strncpy(_ap_passphrase_working, _ap_passphrase, sizeof(_ap_passphrase_working));
  _ap_ip = wifiApIPv4Address();
  _ap_ip_working = _ap_ip;
  _ap_netmask = wifiApIPv4Netmask();
  _ap_netmask_working = _ap_netmask;

  getWifiStaSsid(_sta_ssid, sizeof(_sta_ssid));
  getWifiStaPassphrase(_sta_passphrase, sizeof(_sta_passphrase));

  // register wifi event handlers

  WiFi.onEvent(wifiEventReady, ARDUINO_EVENT_WIFI_READY);
  WiFi.onEvent(wifiEventScanDone, ARDUINO_EVENT_WIFI_SCAN_DONE);
  WiFi.onEvent(wifiEventStaStart, ARDUINO_EVENT_WIFI_STA_START);
  WiFi.onEvent(wifiEventStaStop, ARDUINO_EVENT_WIFI_STA_STOP);
  WiFi.onEvent(wifiEventStaConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(wifiEventStaDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(wifiEventStaAuthmodeChanged, ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE);
  WiFi.onEvent(wifiEventStaGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(wifiEventStaGotIP6, ARDUINO_EVENT_WIFI_STA_GOT_IP6);
  WiFi.onEvent(wifiEventStaLostIP, ARDUINO_EVENT_WIFI_STA_LOST_IP);
  WiFi.onEvent(wifiEventApStart, ARDUINO_EVENT_WIFI_AP_START);
  WiFi.onEvent(wifiEventApStop, ARDUINO_EVENT_WIFI_AP_STOP);
  WiFi.onEvent(wifiEventApStaConnected, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(wifiEventApStaDisconnected, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  WiFi.onEvent(wifiEventApStaIpAssigned, ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
  WiFi.onEvent(wifiEventApProbeReqRecved, ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED);

  // ignoring all ethernet and WPS events on purpose

  // Remove WiFi event
  // Serial.print(F("WiFi Event ID: "));
  // WiFi.removeEvent(_eventIdReady);
}

void wifiLoop()
{

  WifiState nextState = _wifiState;

  switch (_wifiState)
  {

    /*case UNHINGE:
      nextState = UNHINGE;
      break;*/

  case NO_WIFI_YET:
    nextState = handleNoWifiYet();
    break;

  case START_STA_OR_AP:
    nextState = handleStartStaOrAp();
    break;

  case STA_START:
    nextState = handleStaStart();
    break;

  case STA_START_WAIT:
    nextState = handleStaStartWait();
    break;

  case STA_OK:
    nextState = handleStaOk();
    break;

  case STA_FAIL:
    nextState = handleStaFail();
    break;

  case STA_LOST_CONNECTION:
    nextState = handleStaLostConnection();
    break;

  case STA_RECONNECT:
    nextState = handleStaReconnect();
    break;

  case STA_RECONNECT_WAIT:
    nextState = handleStaReconnectWait();
    break;

  case STA_SWITCH_TO_AP:
    nextState = handleStaSwitchToAp();
    break;

  case STA_SWITCH_WAIT_STA_DOWN:
    nextState = handleStaSwitchWaitStaDown();
    break;

  case AP_START:
    nextState = handleApStart();
    break;

  case AP_START_WAIT:
    nextState = handleApStartWait();
    break;

  case AP_OK:
    dnsServer.processNextRequest(); // for captive portal
    nextState = handleApOk();
    break;

  case AP_SWITCH_TO_STA:
    nextState = handleApSwitchToSta();
    break;

  case AP_SWITCH_WAIT_AP_DOWN:
    nextState = handleApSwitchWaitApDown();
    break;

  case AP_FAIL:
    nextState = handleApFail();
    break;

  case NO_WIFI_PERM:
    // dead end
    nextState = NO_WIFI_PERM;
    break;

  default:
    break;
  }

  setState(nextState);
}