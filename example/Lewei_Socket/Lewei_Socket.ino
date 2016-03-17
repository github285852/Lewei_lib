#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "Lewei_socket.h"

ESP8266WiFiMulti wifiMulti;

Lewei_socket LeWei("gateWayNo", "userKey");
unsigned long lastTime = 0;

void LeweiFunc(stuFuncParms &funcParms)
{
    Serial.println(funcParms.FuncName);
    Serial.println(funcParms.Parms[0]);
    Serial.println(funcParms.Parms[1]);
    Serial.println(funcParms.Parms[2]);
    Serial.println(funcParms.Parms[3]);
    Serial.println(funcParms.Parms[4]);
}

void setup()
{
    Serial.begin(115200);	//for output information
	delay(10);
	randomSeed(analogRead(0));

    // wifiMulti.addAP("SSID", "password");
    // wifiMulti.addAP("SSID1", "password1");

    Serial.println("Connecting Wifi...");
	while(wifiMulti.run() == WL_CONNECTED) {
		Serial.println("");
		Serial.println("WiFi connected");
		Serial.println("IP address: ");
		Serial.println(WiFi.localIP());
	}

    LeWei.begin(LeweiFunc);
}

void loop()
{
    if(wifiMulti.run() != WL_CONNECTED) {
		Serial.println("WiFi not connected!");
		delay(1000);
	}
	else
	{
        LeWei.fresh();
        unsigned long ms = millis();
        if(ms - lastTime >= 5000)
        {
            lastTime = ms;

            int randNumber = random(0, 500);
            Serial.println(randNumber);

            LeWei.freshSensor("dust", String(randNumber));
            LeWei.freshSensor("T1", "2");
            LeWei.uploadAllSensor();

            //LeWei.uploadSensor("T1", "3");
        }
	}
    delay(10);
}
