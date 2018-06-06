#include <Arduino.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

#include <ESP8266WiFi.h>

#define YOUR_WIFI_SSID "FIXME-NAT"
#define YOUR_WIFI_PASSWD "hs1337_FIXME"

//#define YOUR_WIFI_SSID "AndroidAP"
//#define YOUR_WIFI_PASSWD "fpwq1879"

#define FIRST_RELAY_PIN D1
#define SECOND_RELAY_PIN D2

int8_t timeZone = 1;
int8_t minutesTimeZone = 0;
bool wifiFirstConnected = false;
int currentRelay;
int lastMinute = -1;

void onSTAConnected(WiFiEventStationModeConnected ipInfo)
{
    Serial.printf("Connected to %s\r\n", ipInfo.ssid.c_str());
}

// Start NTP only after IP network is connected
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
    Serial.printf("Got IP: %s\r\n", ipInfo.ip.toString().c_str());
    Serial.printf("Connected: %s\r\n", WiFi.status() == WL_CONNECTED ? "yes" : "no");
    digitalWrite(LED_BUILTIN, LOW); // Turn on LED
    wifiFirstConnected = true;
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info)
{
    Serial.printf("Disconnected from SSID: %s\n", event_info.ssid.c_str());
    Serial.printf("Reason: %d\n", event_info.reason);
    digitalWrite(LED_BUILTIN, HIGH); // Turn off LED
    NTP.stop();                      // NTP sync can be disabled to avoid sync errors
}

void changeMinute()
{
    //let's flip relays to invert polarity next minute
    if (currentRelay == FIRST_RELAY_PIN)
    {
        currentRelay = SECOND_RELAY_PIN;
    }
    else
    {
        currentRelay = FIRST_RELAY_PIN;
    }
    digitalWrite(currentRelay, LOW);
    delay(1000);
    digitalWrite(currentRelay, HIGH);
    delay(2000);
}

void setup()
{
    // put your setup code here, to run once:

    static WiFiEventHandler e1, e2, e3;

    Serial.begin(9600);
    Serial.println("yo");
    WiFi.mode(WIFI_STA);
    WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);

    pinMode(LED_BUILTIN, OUTPUT);    // Onboard LED
    digitalWrite(LED_BUILTIN, HIGH); // Switch off LED

    e1 = WiFi.onStationModeGotIP(onSTAGotIP); // As soon WiFi is connected, start NTP Client
    e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
    e3 = WiFi.onStationModeConnected(onSTAConnected);

    currentRelay = FIRST_RELAY_PIN;
    pinMode(FIRST_RELAY_PIN, OUTPUT);
    pinMode(SECOND_RELAY_PIN, OUTPUT);

    digitalWrite(FIRST_RELAY_PIN, HIGH);
    digitalWrite(SECOND_RELAY_PIN, HIGH);
}

void loop()
{

    // put your main code here, to run repeatedly:
    static int i = 0;
    static int last = 0;

    if (wifiFirstConnected)
    {
        wifiFirstConnected = false;
        NTP.begin("pool.ntp.org", timeZone, true, minutesTimeZone);
        NTP.setInterval(63);
        lastMinute = minute();
    }

    if ((millis() - last) > 5100)
    {
        //Serial.println(millis() - last);
        last = millis();

        if (minute() != lastMinute)
        {
            Serial.println("Time is " + String(hour()) + " " + String(minute()));
            Serial.println("Changing minute from " + String(lastMinute) + " " + String(minute()));
            changeMinute();
            lastMinute = (lastMinute + 1) % 60;
        }

        Serial.print(i);
        Serial.print(" ");
        Serial.print(NTP.getTimeDateString());
        Serial.print(" ");
        Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
        Serial.print("WiFi is ");
        Serial.print(WiFi.isConnected() ? "connected" : "not connected");
        Serial.print(". ");
        Serial.print("Uptime: ");
        Serial.print(NTP.getUptimeString());
        Serial.print(" since ");
        Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

        i++;
    }
    delay(0);
}