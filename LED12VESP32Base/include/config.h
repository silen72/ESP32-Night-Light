#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <Arduino.h>

/*
    Parameter names for Preference and for the web API
*/

static const char *PrefBrightnessStep = "stbr";
static const char *PrefTransitionDurationMs = "ptdm";
static const char *PrefMaxBrightness = "mbr";
static const char *PrefOnBrightness = "obr";
static const char *PrefNightLightOnDuration = "odu";
static const char *PrefNightLightBrightness = "nlbr";
static const char *PrefMaxNightLightBrightness = "mnlb";
static const char *PrefAllowNightLight = "alnl";
static const char *PrefNightLightLdrThreshold = "nllt";
static const char *PrefMinMovingTargetDistance = "mimd";
static const char *PrefMaxMovingTargetDistance = "mamd";
static const char *PrefMinMovingTargetEnergy = "mime";
static const char *PrefMaxMovingTargetEnergy = "mame";
static const char *PrefMinStationaryTargetDistance = "misd";
static const char *PrefMaxStationaryTargetDistance = "masd";
static const char *PrefMinStationaryTargetEnergy = "mise";
static const char *PrefMaxStationaryTargetEnergy = "mase";

static const char *PrefWifiHostname = "whon";
static const char *PrefWifiApSsid = "wass";
static const char *PrefWifiApPassphrase = "wapa";
static const char *PrefWifiApIpAddress = "waip";
static const char *PrefWifiApNetmask = "wanm";
static const char *PrefWifiStaSsid = "wsss";
static const char *PrefWifiStaPassphrase = "wspa";

static const char *PrefWebAuthUsername = "waun";
static const char *PrefWebAuthPassword = "wapw";

/*
    Default values
*/

static const uint8_t DEFAULT_BRIGHTNESS_STEP = 8;
static const uint16_t DEFAULT_TRANSITION_DURATION_MS = 1000;
static const uint8_t DEFAULT_MAX_BRIGHTNESS = 210;
static const uint8_t DEFAULT_ON_BRIGHTNESS = 210;

static const uint8_t DEFAULT_NIGHTLIGHT_BRIGHTNESS = 16;
static const uint8_t DEFAULT_MAX_NIGHTLIGHT_BRIGHTNESS = 128;
static const bool DEFAULT_ALLOW_NIGHTLIGHT = true;
static const uint16_t DEFAULT_LDR_NIGHTLIGHT_THRESHOLD = 30;
static const uint16_t DEFAULT_NIGHTLIGHT_ON_DURATION_S = 30; // night light stays on for 30s, then checks if it is still needed

static const uint16_t DEFAULT_MIN_MOVING_TARGET_DISTANCE = 0;
static const uint16_t DEFAULT_MAX_MOVING_TARGET_DISTANCE = 300; // UINT16_MAX
static const uint8_t DEFAULT_MIN_MOVING_TARGET_ENERGY = 0;
static const uint8_t DEFAULT_MAX_MOVING_TARGET_ENERGY = UINT8_MAX;
static const uint16_t DEFAULT_MIN_STATIONARY_TARGET_DISTANCE = 0;
static const uint16_t DEFAULT_MAX_STATIONARY_TARGET_DISTANCE = 300; // UINT16_MAX
static const uint8_t DEFAULT_MIN_STATIONARY_TARGET_ENERGY = 0;
static const uint8_t DEFAULT_MAX_STATIONARY_TARGET_ENERGY = UINT8_MAX;

static const char DEFAULT_WIFI_HOSTNAME[] = "lamp";
//static const size_t DEFAULT_WIFI_HOSTNAME_SIZE = 4;
static const char DEFAULT_WIFI_AP_SSID[] = "esp32LEDStrip";
//static const size_t DEFAULT_WIFI_AP_SSID_SIZE = 13;
static const char DEFAULT_WIFI_AP_PASSPHRASE[] = "";
//static const size_t DEFAULT_WIFI_AP_PASSPHRASE_SIZE = 0;
static const uint32_t DEFAULT_WIFI_AP_IP = IPAddress(192, 168, 72, 1);
static const uint32_t DEFAULT_WIFI_AP_NETMASK = IPAddress(255, 255, 255, 0);
static const char DEFAULT_WIFI_STA_SSID[] = "";
//static const size_t DEFAULT_WIFI_STA_SSID_SIZE = 0;
static const char DEFAULT_WIFI_STA_PASSPHRASE[] = "";
//static const size_t DEFAULT_WIFI_STA_PASSPHRASE_SIZE = 0;

static const String DEFAULT_WEB_AUTH_USERNAME = "admin";
static const String DEFAULT_WEB_AUTH_PASSWORD = "lamp";

/*
static const char DEFAULT_MQTT_SERVER[] = "";
static const size_t DEFAULT_MQTT_SERVER_SIZE = 0;
static const char DEFAULT_MQTT_USER[] = "";
static const size_t DEFAULT_MQTT_USER_SIZE = 0;
static const char DEFAULT_MQTT_PASSWORD[] = "";
static const size_t DEFAULT_MQTT_PASSWORD_SIZE = 0;
*/

static const uint8_t MAX_HOSTNAME_LEN = 32;

// Get preference: Night light stays on for this many seconds, then checks if it is still needed
uint16_t nightLightOnDuration();
// Set preference: Night light stays on for this many seconds, then checks if it is still needed
void setNightLightOnDuration(uint16_t value);

// Get preference: Night light is turned on, when the value read from the ldr (-> LDR_PIN) is less than or equal this threshold (0 .. 4095, 0 = dark, 4095  = full brightness)
uint16_t nightLightThreshold();
// Set preference: Night light is turned on, when the value read from the ldr (-> LDR_PIN) is less than or equal this threshold (0 .. 4095, 0 = dark, 4095  = full brightness)
void setNightLightThreshold(uint16_t value);

// Get preference: Brightness of the LED strip in night light mode (0..255, 0 = off, 255 = full on)
uint8_t nightLightBrightness();
// Set preference: Brightness of the LED strip in night light mode (0..255, 0 = off, 255 = full on)
void setNightLightBrightness(uint8_t value);

// Get preference: Limit LED strip brightness (0..255, 0 = off, 255 = full on)
uint8_t maxNightLightBrightness();
// Set preference: Limit LED strip brightness (0..255, 0 = off, 255 = full on)
void setMaxNightLightBrightness(uint8_t value);

// Get preference: Whether night light mode is enabled
bool allowNightLight();
// Set preference: Enable / disable the night light mode
void setAllowNightLight(bool value);

// Get preference: Maximum allowed LED strip brightness (0..255, 0 = off, 255 = full on)
uint8_t maxBrightness();
// Set preference: Maximum allowed LED strip brightness (0..255, 0 = off, 255 = full on)
void setMaxBrightness(uint8_t value);

// Get preference: LED strip brightness when switched on (0..255, 0 = off, 255 = full on)
uint8_t onBrightness();
// Set preference: LED strip brightness when switched on (0..255, 0 = off, 255 = full on)
void setOnBrightness(uint8_t value);

// Get preference: The duration of a transition between two LED strip brightness states
uint16_t transitionDurationMs();
// Set preference: The duration of a transition between two LED strip brightness states
void setTransitionDurationMs(uint16_t value);

// Get preference: Brightness will be increased and decreased by this value (possible values: 1 .. 255)
uint8_t brightnessStep();
// Set preference: Brightness will be increased and decreased by this value (possible values: 1 .. 255, 0 will be treated as 1)
void setBrightnessStep(uint8_t value);

// Get preference for presence detection: A stationary target must be at least this far away from the sensor (in cm) to be considered to enable the night light
uint16_t minStationaryTargetDistance();
// Set preference for presence detection: A stationary target must be at least this far away from the sensor (in cm) to be considered to enable the night light
void setMinStationaryTargetDistance(uint16_t value);

// Get preference for presence detection: A stationary target must be at most this far away from the sensor (in cm) to be considered to enable the night light
uint16_t maxStationaryTargetDistance();
// Set preference for presence detection: A stationary target must be at most this far away from the sensor (in cm) to be considered to enable the night light
void setMaxStationaryTargetDistance(uint16_t value);

// Get preference for presence detection: A stationary target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t minStationaryTargetEnergy();
// Set preference for presence detection: A stationary target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMinStationaryTargetEnergy(uint8_t value);

// Get preference for presence detection: A stationary target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t maxStationaryTargetEnergy();
// Set preference for presence detection: A stationary target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMaxStationaryTargetEnergy(uint8_t value);

// Get preference for presence detection: A moving target must be at least this far away from the sensor (in cm) to be considered to enable the night light
uint16_t minMovingTargetDistance();
// Set preference for presence detection: A moving target must be at least this far away from the sensor (in cm) to be considered to enable the night light
void setMinMovingTargetDistance(uint16_t value);

// Get preference for presence detection: A moving target must be at most this far away from the sensor (in cm) to be considered to enable the night light
uint16_t maxMovingTargetDistance();
// Set preference for presence detection: A moving target must be at most this far away from the sensor (in cm) to be considered to enable the night light
void setMaxMovingTargetDistance(uint16_t value);

// Get preference for presence detection: A moving target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t minMovingTargetEnergy();
// Set preference for presence detection: A moving target must have at least this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMinMovingTargetEnergy(uint8_t value);

// Get preference for presence detection: A moving target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
uint8_t maxMovingTargetEnergy();
// Set preference for presence detection: A moving target must have at most this "energy" (confidence) (0 .. 100) to be considered to enable the night light
void setMaxMovingTargetEnergy(uint8_t value);

// Get preference: Hostname of the lamp device 
String getWifiHostname();
// Set preference: Hostname of the lamp device
void setWifiHostname(const String &value);

// Get preference: SSID of the access point when the lamp device opens up an AP
String getWifiApSsid(); // size_t getWifiApSsid(char *value, uint8_t maxLen);
// Set preference: SSID of the access point when the lamp device opens up an AP
void setWifiApSsid(const String &value); // void setWifiApSsid(const char *value, uint8_t maxLen);

// Get preference: Passphrase of the access point when the lamp device opens up an AP (might be an empty string)
String getWifiApPassphrase(); //size_t getWifiApPassphrase(char *value, uint8_t maxLen);
// Set preference: Passphrase of the access point. A valid passphrase must have at least eight characters. Will set the value and return true when the requirement is met. Will not save the value and return false otherwise.
void setWifiApPassphrase(const String &value); // bool setWifiApPassphrase(const char *value, uint8_t maxLen);

// Get preference: IPv4 address of the access point when the lamp device opens up an AP
uint32_t wifiApIPv4Address();
// Set preference: IPv4 address of the access point when the lamp device opens up an AP
void setWifiAPpIPv4Address(uint32_t value);

// Get preference: IPv4 net mask of the access point when the lamp device opens up an AP
uint32_t wifiApIPv4Netmask();
// Set preference: IPv4 net mask of the access point when the lamp device opens up an AP
void setWifiAPpIPv4Netmask(uint32_t value);

// Get preference: The device will try to connect to this SSID in STAtion mode.
String getWifiStaSsid();
// Set preference: The device will try to connect to this SSID in STAtion mode.
void setWifiStaSsid(const String &value);

// Get preference: The device will try to connect to an existing WLAN with this passphrase in STAtion mode.
String getWifiStaPassphrase();
// Set preference: The device will try to connect to an existing WLAN with this passphrase in STAtion mode.
void setWifiStaPassphrase(const String &value);

String getWebAuthPassword();
void setWebAuthPassword(const String &value);

String getWebAuthUsername();
void setWebAuthUsername(const String &value);


void factoryReset();

// Configure the handling of preferences and configurations
void configSetup();
// Does nothing as of yet
void configLoop();

#endif