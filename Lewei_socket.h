#ifndef __LEWEI_SOCKET_LIB__
#define __LEWEI_SOCKET_LIB__

#include <WiFiClient.h>
#include <aJSON.h> //https://github.com/interactive-matter/aJson
#include <map>

//ESP8266 for Arduino IDE (xtensa-lx106-elf-gcc) and std::map linking error
//
// For some reason the standard ESP build does not link against libstdc++.
// You'll need to edit platforms.txt in $ARDUINO_IDE/hardware/esp8266com/esp8266, and add -lstdc++ to the following line:
//   compiler.c.elf.libs=-lm -lgcc -lhal -lphy -lnet80211 -llwip -lwpa -lmain -lpp -lsmartconfig -lwps -lcrypto -laxtls
// Arduino IDE only reads this file once at start, so, make sure to restart the IDE before compiling.
//
// From: http://stackoverflow.com/questions/33450946/esp8266-for-arduino-ide-xtensa-lx106-elf-gcc-and-stdmap-linking-error

struct stuFuncParms
{
    public:
        String FuncName;
        String Parms[5];
};

typedef void (*FUNCTION)(stuFuncParms &funcParms);
typedef std::map<String, String> SensorMap;

class Lewei_socket //lewei long connnection library
{
    public:
        Lewei_socket(const char *gateWayNo, const char *userKey);
        void begin(FUNCTION callback = NULL);
        void stop();
        uint8_t status();
        void fresh();
        //API
        void uploadSensor(String name, String val);
        void freshSensor(String name, String val);
        void uploadAllSensor();
        void Response(bool success, const char *msg);
        void control(const char *gatewayNo, stuFuncParms &funcParms);
    private:

        String receive();
        void Convert(String content, stuFuncParms &funcParms);
        void FunctionRoute(stuFuncParms &funcParms);
        void ResponseDefault();
        void ResponseData();
        void response(bool success, const char *msg, aJsonObject* data = NULL);
        aJsonObject* CreatItem(const char *name, const char *val);

        void update();
        void sendToServer(aJsonObject* msg);

        SensorMap _sensorMap;
        aJsonObject* ConvertSensorData();
        const char *GateWayNo;
        const char *UserKey;

        WiFiClient client;
        FUNCTION FunctionHandle;
        unsigned long lastUpdate;
        bool enable;
        String revbuf;
        bool _responsed;

};

#endif
