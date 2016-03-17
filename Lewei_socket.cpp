#include "Lewei_socket.h"

const char *LeWei_Host = "tcp.lewei50.com";
const uint16_t LeWei_Port = 9960;
const char *FinishSign = "&^!";

const long interval = 30000; //30s

Lewei_socket::Lewei_socket(const char *gateWayNo, const char *userKey)
:GateWayNo(gateWayNo)
,UserKey(userKey)
,FunctionHandle(NULL)
,enable(false)
,revbuf("")
,_responsed(false)
{

}
void Lewei_socket::begin(FUNCTION callback)
{
    FunctionHandle = callback;
    enable = true;
    if(client) client.stop(); //终止已有连接
    if(!client.connect(LeWei_Host, LeWei_Port))
    {
        Serial.println("connection failed");
        return;
    }
    Serial.println("Lewei_socket connected");
    update();
}
void Lewei_socket::stop()
{
    client.stop();
    enable = false;
}
uint8_t Lewei_socket::status()
{
    if(!client)  return 0;  //空连接返回0
    return client.status(); //非空连接返回状态值，0表示Closed

    // enum tcp_state {
    //   CLOSED      = 0,
    //   LISTEN      = 1,
    //   SYN_SENT    = 2,
    //   SYN_RCVD    = 3,
    //   ESTABLISHED = 4,
    //   FIN_WAIT_1  = 5,
    //   FIN_WAIT_2  = 6,
    //   CLOSE_WAIT  = 7,
    //   CLOSING     = 8,
    //   LAST_ACK    = 9,
    //   TIME_WAIT   = 10
    // };
}

void Lewei_socket::fresh()
{
    if(!status()) //empty or disconnect
    {
        Serial.println("Lewei_socket disconnected");
        if(enable) begin(FunctionHandle); //reconnect
    }
    else //connected
    {
        String content = receive();
        if(content != "")
        {
            Serial.print("Rev : ");Serial.println(content);
            stuFuncParms funcParms;
            Convert(content, funcParms);
            FunctionRoute(funcParms);
        }
        if(millis() - lastUpdate >= interval)   update(); //keep connect
    }
}
String Lewei_socket::receive()
{
    int n = client.available();
    if(n > 0)
    {
        char buf[n];
        client.readBytes(buf, n);
        revbuf += buf;
    }

    int startIndex = revbuf.indexOf('{');
    int endIndex = revbuf.indexOf(FinishSign);
    if((startIndex == -1)||(endIndex == -1)) return "";

    String content = revbuf.substring(startIndex, endIndex);
    revbuf = revbuf.substring(endIndex + 3);

    return content;
}
void Lewei_socket::Convert(String content, stuFuncParms &funcParms)
{
    if(content == "") return;
    int n = content.length() + 1;
    char buf[n];
    content.toCharArray(buf, n);

    aJsonObject* msg = aJson.parse(buf);
    if(msg == NULL) return;
    aJsonObject* f = aJson.getObjectItem(msg, "f");
    if(f == NULL) return;

    funcParms.FuncName = f -> valuestring;
    for(int i = 0; i < 5; i++)
    {
        String key = "p" + String(i + 1);
        aJsonObject* p = aJson.getObjectItem(msg, key.c_str());
        if(p == NULL)  break;
        funcParms.Parms[i] = p -> valuestring;
    }
    aJson.deleteItem(msg);
}


void Lewei_socket::update()
{
    aJsonObject* msg = aJson.createObject();
    aJson.addStringToObject(msg, "method", "update");
    aJson.addStringToObject(msg, "gatewayNo", GateWayNo);
    aJson.addStringToObject(msg, "userkey", UserKey);

    sendToServer(msg);
}
void Lewei_socket::uploadSensor(String name, String val)
{
    aJsonObject* msg = aJson.createObject();
    aJson.addStringToObject(msg, "method", "upload");

    aJsonObject* data = aJson.createArray();
    aJsonObject* item = CreatItem(name.c_str(), val.c_str());
    aJson.addItemToArray(data, item);

    aJson.addItemToObject(msg, "data", data);
    sendToServer(msg);
}
void Lewei_socket::freshSensor(String name, String val)
{
    _sensorMap[name] = val;
}
void Lewei_socket::uploadAllSensor()
{
    aJsonObject* msg = aJson.createObject();
    aJson.addStringToObject(msg, "method", "upload");

    aJsonObject* data = ConvertSensorData();

    aJson.addItemToObject(msg, "data", data);
    sendToServer(msg);
    _sensorMap.clear();
}
aJsonObject* Lewei_socket::ConvertSensorData()
{
    aJsonObject* data = aJson.createArray();
    SensorMap::iterator my_Itr;
    for(my_Itr = _sensorMap.begin(); my_Itr != _sensorMap.end(); ++my_Itr)
    {
        String key = my_Itr->first;
        String value = my_Itr->second;
        aJsonObject* item = CreatItem(key.c_str(), value.c_str());
        aJson.addItemToArray(data, item);
    }
    return data;
}
aJsonObject* Lewei_socket::CreatItem(const char *name, const char *val)
{
    aJsonObject* item = aJson.createObject();
    aJson.addStringToObject(item, "Name", name);
    aJson.addStringToObject(item, "Value", val);
    return item;
}

void Lewei_socket::FunctionRoute(stuFuncParms &funcParms)
{
    _responsed = false;
    if(funcParms.FuncName == "message")             return;
    else if(funcParms.FuncName == "getAllSensors")   ResponseData();
    else
    {
        if(FunctionHandle != NULL)
            FunctionHandle(funcParms);
        if(_responsed == false)
            ResponseDefault();
    }
}
void Lewei_socket::Response(bool success, const char *msg)
{
    response(success, msg);
}
void Lewei_socket::ResponseDefault()
{
    response(false, "Unknown Function");
}
void Lewei_socket::ResponseData()
{
    aJsonObject* data = ConvertSensorData();
    response(true, "getAllSensors", data);
}
void Lewei_socket::response(bool success, const char *message, aJsonObject* data)
{
    aJsonObject* msg = aJson.createObject();
    aJson.addStringToObject(msg, "method", "response");

    aJsonObject* result = aJson.createObject();
    aJson.addBooleanToObject(result, "successful", success);
    aJson.addStringToObject(result, "message", message);
    if(data != NULL) aJson.addItemToObject(result, "data", data);
    aJson.addItemToObject(msg, "result", result);
    sendToServer(msg);
    _responsed = true;
}
void Lewei_socket::control(const char *gatewayNo, stuFuncParms &funcParms)
{
    aJsonObject* msg = aJson.createObject();
    aJson.addStringToObject(msg, "method", "control");
    aJson.addStringToObject(msg, "gatewayNo", gatewayNo);
    aJson.addStringToObject(msg, "f", funcParms.FuncName.c_str());
    for(int i = 0; i < 5; i++)
    {
        String parm = funcParms.Parms[i];
        if(parm == "") break;
        String parmName = "p" + String(i + 1);
        aJson.addStringToObject(msg, parmName.c_str(), parm.c_str());
    }
    sendToServer(msg);
}
void Lewei_socket::sendToServer(aJsonObject* msg)
{
    String buf = aJson.print(msg);
    aJson.deleteItem(msg);

    buf = buf + FinishSign;
    client.write(buf.c_str(), buf.length());
    Serial.print("Send: ");Serial.println(buf);

    lastUpdate = millis();
}
