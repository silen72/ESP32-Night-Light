#include <ld2410.h>
#include "presence.h"

ld2410 radar;

uint32_t lastReading = 0;
bool radarConnected = false;

LD2410Firmware firmwareInfo()
{
    LD2410Firmware fwInfo;
    fwInfo.Major = radar.firmware_major_version;
    fwInfo.Minor = radar.firmware_minor_version;
    fwInfo.Bugfix = radar.firmware_bugfix_version;
    return fwInfo;
}

LD2410Detection presenceInfo()
{
    LD2410Detection detect;
    detect.presenceDetected = radar.presenceDetected();
    detect.movingTargetDetected = radar.movingTargetDetected();
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
    return detect;
}

bool isConnected() { return radar.isConnected(); }
bool presenceDetected() { return radar.presenceDetected(); }
bool stationaryTargetDetected() { return radar.stationaryTargetDetected(); }
bool movingTargetDetected() { return radar.movingTargetDetected(); }

void presenceSetup()
{
    radar.debug(MONITOR_SERIAL);                                        // Uncomment to show debug information from the library on the Serial Monitor. By default this does not show sensor reads as they are very frequent.
    RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN); // UART for monitoring the radar

    delay(500);
    MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));

    if (radar.begin(RADAR_SERIAL))
    {
        MONITOR_SERIAL.println(F("OK"));
        MONITOR_SERIAL.print(F("LD2410 firmware version: "));
        MONITOR_SERIAL.print(radar.firmware_major_version);
        MONITOR_SERIAL.print('.');
        MONITOR_SERIAL.print(radar.firmware_minor_version);
        MONITOR_SERIAL.print('.');
        MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
    }
    else
    {
        MONITOR_SERIAL.println(F("not connected"));
    }
}

void presenceLoop()
{
    radar.read();
    if (radar.isConnected() && millis() - lastReading > 1000) // Report every 1000ms
    {
        lastReading = millis();
        if (radar.presenceDetected())
        {
            if (radar.stationaryTargetDetected())
            {
                MONITOR_SERIAL.print(F("Stationary target: "));
                MONITOR_SERIAL.print(radar.stationaryTargetDistance());
                MONITOR_SERIAL.print(F("cm energy:"));
                MONITOR_SERIAL.print(radar.stationaryTargetEnergy());
                MONITOR_SERIAL.print(' ');
            }
            if (radar.movingTargetDetected())
            {
                MONITOR_SERIAL.print(F("Moving target: "));
                MONITOR_SERIAL.print(radar.movingTargetDistance());
                MONITOR_SERIAL.print(F("cm energy:"));
                MONITOR_SERIAL.print(radar.movingTargetEnergy());
            }
            MONITOR_SERIAL.println();
        }
        else
        {
            MONITOR_SERIAL.println(F("No target"));
        }
    }
}
