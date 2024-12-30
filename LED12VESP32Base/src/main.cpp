#include <Arduino.h>
#include <WebServer.h>

#include "touch.h"
#include "ldr.h"
#include "led_strip.h"
#include "config.h"
#include "presence.h"

enum State {
  ON,
  NIGHT_LITE,
  OFF
};

unsigned long lastReport = 0;
bool fwDone = true;
bool cfgDone = true;
State state = NIGHT_LITE;

void setup()
{
  MONITOR_SERIAL.begin(115200);
  MONITOR_SERIAL.println(F("Start"));

  configSetup();
  ldrSetup();
  touchSetup();
  presenceSetup();
  ledStripSetup();
}

void loop()
{
  configLoop();

  bool allowNL = allowNightLight();

  ldrLoop();
  uint16_t ldrValue = averagedBrightness();
  bool darkEnoughForNightLight = allowNL && ldrValue <= nightLightThreshold();
  bool enableNightLight = false;

  touchLoop();
  presenceLoop();
  ledStripLoop();

  unsigned long now = millis();
  // emit a status report every second
  if (now - lastReport > 1000)
  {
    lastReport = now;

    MONITOR_SERIAL.print(F("ldr: "));
    MONITOR_SERIAL.print(ldrValue);
    MONITOR_SERIAL.print(F("dark enough for night light: "));
    MONITOR_SERIAL.println(darkEnoughForNightLight);

    LD2410Detection prsInfo = presenceInfo();

    enableNightLight = darkEnoughForNightLight && prsInfo.presenceDetected && state == NIGHT_LITE;
    MONITOR_SERIAL.print(F("enablge night light: "));
    MONITOR_SERIAL.println(enableNightLight);

    if (prsInfo.presenceDetected)
    {
      if (prsInfo.stationaryTargetDetected)
      {
        MONITOR_SERIAL.print(F("Stationary target: "));
        MONITOR_SERIAL.print(prsInfo.stationaryTargetDistance);
        MONITOR_SERIAL.print(F("cm energy:"));
        MONITOR_SERIAL.print(prsInfo.stationaryTargetEnergy);
        MONITOR_SERIAL.print(' ');
      }
      if (prsInfo.movingTargetDetected)
      {
        MONITOR_SERIAL.print(F("Moving target: "));
        MONITOR_SERIAL.print(prsInfo.movingTargetDistance);
        MONITOR_SERIAL.print(F("cm energy:"));
        MONITOR_SERIAL.print(prsInfo.movingTargetEnergy);
      }
      MONITOR_SERIAL.println();
    }
    else
    {
      MONITOR_SERIAL.println(F("No target"));
    }

    if (!cfgDone)
    {
      LD2410Config presenceCfg = currentConfig();
      if (presenceCfg.Valid)
      {
        MONITOR_SERIAL.print(F("LD2410 config: \nmax_gate:"));
        MONITOR_SERIAL.println(presenceCfg.max_gate);
        MONITOR_SERIAL.print(F("max_moving_gate: "));
        MONITOR_SERIAL.println(presenceCfg.max_moving_gate);
        MONITOR_SERIAL.print(F("max_stationary_gate: "));
        MONITOR_SERIAL.println(presenceCfg.max_stationary_gate);
        MONITOR_SERIAL.print(F("sensor_idle_time: "));
        MONITOR_SERIAL.println(presenceCfg.sensor_idle_time);

        MONITOR_SERIAL.print(F("motion_sensitivity: "));
        for (uint8_t i = 0; i < presenceCfg.max_moving_gate; i++)
        {
          MONITOR_SERIAL.print(presenceCfg.motion_sensitivity[i]);
          if (i < presenceCfg.max_moving_gate - 1)
            MONITOR_SERIAL.print(F(", "));
        }
        MONITOR_SERIAL.println();

        MONITOR_SERIAL.print(F("motion_sensitivity: "));
        for (uint8_t i = 0; i < presenceCfg.max_moving_gate; i++)
        {
          MONITOR_SERIAL.print(presenceCfg.stationary_sensitivity[i]);
          if (i < presenceCfg.max_moving_gate - 1)
            MONITOR_SERIAL.print(F(", "));
        }
        MONITOR_SERIAL.println();
        cfgDone = true;
      }
    }
    else
    {
      MONITOR_SERIAL.print(F("No config yet"));
    }

    if (!fwDone)
    {
      LD2410Firmware fwInfo = firmwareInfo();
      if (fwInfo.Valid)
      {
        fwDone = true;
        MONITOR_SERIAL.print(F("LD2410 firmware: v"));
        MONITOR_SERIAL.print(fwInfo.Major);
        MONITOR_SERIAL.print('.');
        MONITOR_SERIAL.print(fwInfo.Minor);
        MONITOR_SERIAL.print('.');
        MONITOR_SERIAL.println(fwInfo.Bugfix);
      }
    }
    else
    {
      MONITOR_SERIAL.print(F("No firmware info yet"));
    }
  }
}
