#ifndef _WIFIHANDLER_H_
#define _WIFIHANDLER_H_

#include <Arduino.h>

// The WiFi mode the device is supposed to be in
enum WifiMode
{
    WifiMode_OFF,   // WiFi is switched off. Should be seem only directly after power on and in between mode changes.
    WifiMode_AP,    // The device serves as an Access Point. Normally only after the very first power on. Can be requested via control unit.
    WifiMode_STA    // The device is (or at least trying to be) connected to a WiFi.
};

// The result of trying to start the WifiMode
enum ModeResult
{
    MODE_NOT_ATTEMPTED_YET, // Starting this mode has not been attempted since last configuration change or power up.
    MODE_SUCCESS,           // This mode has been successfully started at least once since last configuration change or power up.
    MODE_FAIL               // An attempt to start this mode has failed. Change configuration and try again.
};

enum WifiState
{
    //UNHINGE,                  // DEBUG: stop at this state
    NO_WIFI_YET,              // No connection attempt has been made yet. This is the initial state of the device. Next state is START_STA_OR_AP.
    START_STA_OR_AP,          // Decide whether to start WiFi as STAtion or as Access Point (depending on configuration). Next state is either STA_START or AP_START.
    STA_START,                // Start WiFi as STAtion. Next state is STA_START_WAIT.
    STA_START_WAIT,           // STA started but not yet successfully connected. Next state depends on the success of the connection attempt: either STA_START_WAIT, STA_OK or STA_FAIL.
    STA_OK,                   // Successfully connected to the specified WiFi. Checks connection repeatedly. Next state depends on the check result: either STA_OK or STA_LOST_CONNECTION.
    STA_FAIL,                 // Connecting to the specified WiFi was not successful. Next state is AP_START.
    STA_LOST_CONNECTION,      // Lost a formerly successful connection. Next state is STA_RECONNECT.
    STA_RECONNECT,            // Start reconnecting to the WiFi. Next state is STA_RECONNECT_WAIT.
    STA_RECONNECT_WAIT,       // Reconnecting started but not yet successful. Will continue trying infinitely. Next state depends on the success of the reconnection attempt: either STA_RECONNECT_WAIT or STA_OK.
    STA_SWITCH_TO_AP,         // Commands the device to switch from STA to AP. Starts the shut down of the STA connection. This can only be triggered by external events (via web api, web interface, MQTT or manual). Next state is STA_SWITCH_WAIT_AP_DOWN.
    STA_SWITCH_WAIT_STA_DOWN, // Shutting down the STA connection started but not complete. Checks connection repeatedly. Next state depends on the check result: either STA_SWITCH_WAIT_STA_DOWN or AP_START
    AP_START,                 // Start WiFi as Access Point. Next state is AP_START_WAIT.
    AP_START_WAIT,            // AP started but not yet up completely. Next state depends: either AP_START_WAIT, AP_OK or AP_FAIL.
    AP_OK,                    // Successfully started the access point. Stays in this state infinitely.
    AP_SWITCH_TO_STA,         // Commands the device to switch from AP to STA. Starts the shut down of the access point. This can only be triggered by external events (via web api, web interface, MQTT or manual). Next state is AP_SWITCH_WAIT_AP_DOWN.
    AP_SWITCH_WAIT_AP_DOWN,   // Shutting down the access point started but not complete. Checks repeatedly. Next state depends on the check result: either AP_SWITCH_WAIT_AP_DOWN or STA_START
    AP_FAIL,                  // Setting up the access point has failed. Next state is NO_WIFI_YET.
    NO_WIFI_PERM,             // Attempts for both STA and AP have failed. No way to recover from this, so stay offline. Next state is NO_WIFI_PERM. This is a BUG!
};

struct WifiStateInfo
{
    WifiMode mode;
    ModeResult apModeResult;
    ModeResult staModeResult;
    WifiState currentState;
    unsigned long enteredStateAtTs;
};

WifiStateInfo wifiCurrentState();

void wifiDebug(Stream &terminalStream);

void requestAPMode();

void wifiSetup();
void wifiLoop();

#endif