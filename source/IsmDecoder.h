
/* IsmDecoder: library for ESP8266 (for now testet only on WeMos D1 R1) which implements 
   an gateway between and 433MHz radio devices and cloud.

   This library uses in its implementation some functions from a modified version of 
   rc-swich library ( https://github.com/sui77/rc-switch/ )

*/


#ifndef IsmDecoder_h
#define IsmDecoder_h

#define DATA_SAMPLE_SIZE_MAX 4
#define RAW_DATA_MAX_LENGTH 100
#define MAX_BIT_LENGTH 37



#include <inttypes.h>
#include "Arduino.h"
#include "RcSwitchChanged.h"
#include "ESP8266WiFi.h"
#include <string.h>
#include <math.h>
#include <EEPROM.h>
#include <WiFiClient.h> 

using namespace std;

static const uint16_t defaultPort = 80;
static const int timeTollerance = 150;
static int  OOK_data[RAW_DATA_MAX_LENGTH];
static int  bitData[MAX_BIT_LENGTH];
static const char *configServerSsid = "GatewayConfig";
static const char *configServerPassword = "password";
static const char *form="<!DOCTYPE html> <html> <head> <style> body { background-color: #AFEEEE; font-family: Arial, Helvetica, sans-serif; } ul{ list-style-type: none; margin-left: 2%; } .box{ background-color: white; width: 450px; border-style: solid; border-width: 1px; border-color: black; } .input{ float:right; margin-right:10%; } .limember{ margin-top:15px; margin-left:-4%; } </style> </head> <body> <ul> <li><h1>IsmGateWay Configuration Pannel</h1></li> <li> <div class=\"box\"> <form method=\"GET\" > <ul > <li class=\"limember\"><b><p style=\"font-size: 20px;\">Router configuration:</p></b> </li> <li class=\"limember\"><b>Router SSID</b>: <input class=\"input\" type=\"text\" id=\"ssid\" name=\"ssid\" /> </li> <li class=\"limember\"><b>Router Password</b>: <input class=\"input\" type=\"password\" id=\"pass\" name=\"pass\"/> </li> <li class=\"limember\"><input class=\"number\" style=\"display:none;\" type=\"password\" id=\"nrOfFields\" name=\"nrOfFields\"/> </li> </ul> <br><br> </div><br></li> <li><div class=\"box\" style=\"width:850px;\"> <ul> <li class=\"limember\"><b><p style=\"font-size: 20px;\">ThingSpeak Fields Config:</p></b> </li> <li><button type=\"button\" style=\"margin-left:-17px\" onclick=\"addField()\" >Add Field</button><button type=\"button\" style=\"margin-left:3px\" onclick=\"removeField()\">Remove Field</button></li><br> <li> <table style=\"width:100%;margin-left:-17px\"> <tr id=\"field1\"> <td>Field Name: <input class=\"input\" type=\"text\" id=\"fldName1\" name=\"fldName1\" style=\"float:none;\" /><br> <br></td> <td>Channel API: <input class=\"input\" type=\"text\" id=\"api1\" name=\"api1\" style=\"float:none;\" /><br> <br></td> <td>Device Protocol: <select name = \"prot1\" id = \"prot1\"> <option value = \"2\" selected>Prologue</option> <option value = \"1\">Nexus</option> </select> <br> <br> </td> <td>RF Channel: <select name = \"ch1\" id = \"ch1\"> <option value = \"0\" selected>channel 1</option> <option value = \"1\">channel 2</option> <option value = \"2\">channel 3</option> </select> <br> <br> </td> <td>Data Type: <select name = \"dt1\" id = \"dt1\"> <option value = \"1\" selected>Temperature (C)</option> <option value = \"2\">Humidity (%RH)</option> <option value = \"3\">BatteryStatus (high/low)</option> </select> <br> <br> </td> </tr> <tr id=\"field2\"> <td>Field Name: <input class=\"input\" type=\"text\" id=\"fldName2\" name=\"fldName2\" style=\"float:none;\" /><br> <br></td> <td>Channel API: <input class=\"input\" type=\"text\" id=\"api2\" name=\"api2\" style=\"float:none;\" /><br> <br></td> <td>Device Protocol: <select name = \"prot2\" id = \"prot2\"> <option value = \"2\" selected>Prologue</option> <option value = \"1\">Nexus</option> </select> <br> <br> </td> <td>RF Channel: <select name = \"ch2\" id = \"ch2\"> <option value = \"0\" selected>channel 1</option> <option value = \"1\">channel 2</option> <option value = \"2\">channel 3</option> </select> <br> <br> </td> <td>Data Type: <select name = \"dt2\" id = \"dt2\"> <option value = \"1\" selected>Temperature (C)</option> <option value = \"2\">Humidity (%RH)</option> <option value = \"3\">BatteryStatus (high/low)</option> </select> <br> <br> </td> </tr> <tr id=\"field3\"> <td>Field Name: <input class=\"input\" type=\"text\" id=\"fldName3\" name=\"fldName3\" style=\"float:none;\" /><br> <br></td> <td>Channel API: <input class=\"input\" type=\"text\" id=\"api3\" name=\"api3\" style=\"float:none;\" /><br> <br></td> <td>Device Protocol: <select name = \"prot3\" id = \"prot3\"> <option value = \"2\" selected>Prologue</option> <option value = \"1\">Nexus</option> </select> <br> <br> </td> <td>RF Channel: <select name = \"ch3\" id = \"ch3\"> <option value = \"0\" selected>channel 1</option> <option value = \"1\">channel 2</option> <option value = \"2\">channel 3</option> </select> <br> <br> </td> <td>Data Type: <select name = \"dt3\" id = \"dt3\"> <option value = \"1\" selected>Temperature (C)</option> <option value = \"2\">Humidity (%RH)</option> <option value = \"3\">BatteryStatus (high/low)</option> </select> <br> <br> </td> </tr> <tr id=\"field4\"> <td>Field Name: <input class=\"input\" type=\"text\" id=\"fldName4\" name=\"fldName4\" style=\"float:none;\" /><br> <br></td> <td>Channel API: <input class=\"input\" type=\"text\" id=\"api4\" name=\"api4\" style=\"float:none;\" /><br> <br></td> <td>Device Protocol: <select name = \"prot4\" id = \"prot4\"> <option value = \"2\" selected>Prologue</option> <option value = \"1\">Nexus</option> </select> <br> <br> </td> <td>RF Channel: <select name = \"ch4\" id = \"ch4\"> <option value = \"0\" selected>channel 1</option> <option value = \"1\">channel 2</option> <option value = \"2\">channel 3</option> </select> <br> <br> </td> <td>Data Type: <select name = \"dt4\" id = \"dt4\"> <option value = \"1\" selected>Temperature (C)</option> <option value = \"2\">Humidity (%RH)</option> <option value = \"3\">BatteryStatus (high/low)</option> </select> <br> <br> </td> </tr> <tr id=\"field5\"> <td>Field Name: <input class=\"input\" type=\"text\" id=\"fldName5\" name=\"fldName5\" style=\"float:none;\" /><br> <br></td> <td>Channel API: <input class=\"input\" type=\"text\" id=\"api5\" name=\"api5\" style=\"float:none;\" /><br> <br></td> <td>Device Protocol: <select name = \"prot5\" id = \"prot5\"> <option value = \"2\" selected>Prologue</option> <option value = \"1\">Nexus</option> </select> <br> <br> </td> <td>RF Channel: <select name = \"ch5\" id = \"ch5\"> <option value = \"0\" selected>channel 1</option> <option value = \"1\">channel 2</option> <option value = \"2\">channel 3</option> </select> <br> <br> </td> <td>Data Type: <select name = \"dt5\" id = \"dt5\"> <option value = \"1\" selected>Temperature (C)</option> <option value = \"2\">Humidity (%RH)</option> <option value = \"3\">BatteryStatus (high/low)</option> </select> <br> <br> </td> </tr> </table> </li> <li id=\"warning\"> <p style=\"color:red;\">[WARNING] No more than 5 data fields are allowed !</p> </li> </ul> </div> </li> <li > <input style=\"margin-left:3px;\" class=\"limember\" type=\"submit\" value=\"Apply Changes\" id=\"apply\" /> </li> </ul> <br> </form> <script> function disableInput(index){ document.getElementById(\"fldName\" + index).disabled = true; document.getElementById(\"api\" + index).disabled = true; document.getElementById(\"prot\" + index).disabled = true; document.getElementById(\"ch\" + index).disabled = true; document.getElementById(\"dt\" + index).disabled = true; } function enableInput(index){ document.getElementById(\"fldName\" + index).disabled = false; document.getElementById(\"api\" + index).disabled = false; document.getElementById(\"prot\" + index).disabled = false; document.getElementById(\"ch\" + index).disabled = false; document.getElementById(\"dt\" + index).disabled = false; } var nrOfFields=1; document.getElementById(\"nrOfFields\").value = nrOfFields; document.getElementById(\"warning\").style.display= \"none\"; for(i=2;i<=5;i++){ var fieldId=\"field\"+i; document.getElementById(fieldId).style.display = \"none\"; disableInput(i); } function refreshFields(){ for(var i=2;i<=5;i++){ var fieldId=\"field\"+i; if(nrOfFields>=i){ document.getElementById(fieldId).style.display = \"table-row\"; enableInput(i); } else{ document.getElementById(fieldId).style.display= \"none\"; disableInput(i); } } } function addField(){ if(nrOfFields<5){ nrOfFields+=1; document.getElementById(\"nrOfFields\").value = nrOfFields; refreshFields(); document.getElementById(\"warning\").style.display= \"none\"; } else{ document.getElementById(\"warning\").style.display= \"list-item\"; } } function removeField(){ document.getElementById(\"warning\").style.display= \"none\"; if(nrOfFields>1){ nrOfFields-=1; document.getElementById(\"nrOfFields\").value = nrOfFields; refreshFields(); } } </script> </body> </html>";


enum protocolMode{
    basic,extended
};
enum networkMode{
    online,offline
};

enum workingMode{
    normal,config
};


enum dataType{
    
undefined,signedTempCelsius,humidityRh,batteryStatus,channel,button

};
struct dataPosition{
    int startBit;
    int stopBit;
};

struct protocol{
    char* name;
    int bitLength;
    int zeroHighTime;
    int zeroLowTime;
    int oneHighTime;
    int oneLowTime;
    int dataLength;
    dataType dataTypes[DATA_SAMPLE_SIZE_MAX];
    dataPosition dataPos[DATA_SAMPLE_SIZE_MAX];
    
};

struct data{
    dataType type;
    float value;
    
};

struct dataSet{
    int protocolNumber;
    data dataVector[DATA_SAMPLE_SIZE_MAX];
    bool newData;
};
struct tsField{
    //int channelIndex;
    String fieldName;
    int protocolIndex;
    int channel;
    dataType dataTypeIndex;

};
struct tsChannel{
    String channelSendApi;
    int nrOfFields;
    tsField* fields;
    int currentIndex;
};



static const protocol protocolList[] = {

{"ControllerTest",24,500,1500,1500,500,1, {button,undefined,undefined,undefined} ,{{ 16,23 },{ -1,-1 },{ -1,-1 },{ -1,-1 }}},
{"Nexus",36,500,1000,500,2000,4,{ batteryStatus,channel,signedTempCelsius,humidityRh },{{8,8},{10,11},{12,23},{28,35}}},
{"Prologue",37,500,2000,500,4000,4,{ batteryStatus,channel,signedTempCelsius,humidityRh },{ {12 , 12},{ 14, 15},{16 ,27 },{28 ,35 } } }

};
static const int nOfProtocols = 3;

static int resetPin=D13;
static workingMode wMode=normal;

int protocolIdentification(int,int);


class GateWay{

    RCSwitch mySwitch;
    RCSwitch mySwitchExtended;
    WiFiServer server =  WiFiServer(defaultPort);
    String routerName ;
    String routerPassword;
    char*  host;

    protocolMode protocolSet;
    networkMode mode;
    
    int interruptPin;
    int interruptPinExtended;
    
    int changeModePin;

    int nrOfTsChannels;
    tsChannel* tsChannels;

    
    

    dataSet lastReceivedData;

    void decode();
    void identifyData();
    float setSpecificData(char[],dataType,dataPosition);
    void serialPrintData();
    char* getTypeAsString(dataType);
    bool checkIfValidData(int,int);
    dataType intToDataType(int);

    public:

  

    GateWay(int,int,int);
    GateWay(int,int,int,int);

    void enable();
    bool receive();
    bool sendToThingSpeak();
    void handleConfig();
    void applyConfig();

    data getTemperature();
    data getHumidity();
    data getButton();
    data getBatteryStatus();
    data getChannel();
    float getData(dataType);
    

    





    
    

    
};

void IRAM_ATTR swapWorkingModes();  



#endif