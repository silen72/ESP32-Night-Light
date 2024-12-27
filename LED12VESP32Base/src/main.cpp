#include <Arduino.h>
#include <WebServer.h>

#include "touch.h"
#include "ldr.h"
#include "led_strip.h"
#include "config.h"
#include "presence.h"

unsigned long lastReport = 0;

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

  ldrLoop();

  touchLoop();

  presenceLoop();
  
  ledStripLoop();

  unsigned long now = millis();
  if (now - lastReport > 1000)
  {
    MONITOR_SERIAL.print(F("ldr: "));
    MONITOR_SERIAL.println(averagedBrightness());
    lastReport = now;

    LD2410Detection prsInfo = presenceInfo();

  }
}
