#include <ld2410.h>
#include "presence.h"

ld2410 radar;

unsigned long const SETUP_DELAY_MS = 1500;

uint32_t lastReading = 0;
bool radarConnected = false;
unsigned long setupTs = 0;
bool setupDone = false;

Stream *debug_uart_presence = nullptr;

LD2410Firmware firmwareInfo()
{
  LD2410Firmware fwInfo;
  if (setupDone && radar.requestFirmwareVersion())
  {
    fwInfo.Valid = true;
    fwInfo.Major = radar.firmware_major_version;
    fwInfo.Minor = radar.firmware_minor_version;
    fwInfo.Bugfix = radar.firmware_bugfix_version;
  }
  return fwInfo;
}

LD2410Detection presenceInfo()
{
  LD2410Detection detect;
  if (setupDone)
  {
    detect.presenceDetected = radar.presenceDetected();
    detect.movingTargetDetected = radar.movingTargetDetected();
    detect.stationaryTargetDetected = radar.stationaryTargetDetected();
    if (detect.movingTargetDetected)
    {
      detect.movingTargetDistance = radar.movingTargetDistance();
      detect.movingTargetEnergy = radar.movingTargetEnergy();
    }
    if (detect.stationaryTargetDetected)
    {
      detect.stationaryTargetDistance = radar.stationaryTargetDistance();
      detect.stationaryTargetEnergy = radar.stationaryTargetEnergy();
    }
  }
  return detect;
}

LD2410Config currentConfig()
{
  LD2410Config config;
  config.Valid = setupDone && radar.requestCurrentConfiguration();
  if (config.Valid)
  {
    config.max_gate = radar.max_gate;
    config.max_moving_gate = radar.max_moving_gate;
    config.max_stationary_gate = radar.max_stationary_gate;
    for (uint8_t i = 0; i < 9; i++)
    {
      config.motion_sensitivity[i] = radar.stationary_sensitivity[i];
      config.stationary_sensitivity[i] = radar.stationary_sensitivity[i];
    }
    config.sensor_idle_time = radar.sensor_idle_time;
  }
  return config;
}

bool isConnected() { return radar.isConnected(); }

bool requestFactoryReset()
{
  return radar.requestFactoryReset();
}

void presenceDebug(Stream &terminalStream)
{
  debug_uart_presence = &terminalStream;
}

void presenceSetup()
{
  // radar.debug(MONITOR_SERIAL);                                     // Uncomment to show debug information from the library on the Serial Monitor. By default this does not show sensor reads as they are very frequent.
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN); // UART for monitoring the radar
  setupTs = millis();
}

void presenceLoop()
{
  if (setupDone)
  {
    radar.read();
  }
  else // delay reading the sensor for a few milliseconds
  {
    setupDone = ((millis() - setupTs) > SETUP_DELAY_MS);
    if (setupDone)
    {
      if (debug_uart_presence != nullptr)
      {
        debug_uart_presence->println(F("start radar.begin"));
      }
      radar.begin(RADAR_SERIAL, true);
      if (debug_uart_presence != nullptr)
      {
        debug_uart_presence->println(F("radar.begin done"));
      }
    }
  }
}
