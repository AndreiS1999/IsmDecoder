
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

using namespace std;

static const uint16_t defaultPort = 80;
static const int timeTollerance = 150;
static int  OOK_data[RAW_DATA_MAX_LENGTH];
static int  bitData[MAX_BIT_LENGTH];

enum protocolMode{
    basic,extended
};
enum networkMode{
    online,offline
};


enum dataType{
    
undefined,signedTempCelsius,batteryStatus,humidityRh,channel,button

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



static const protocol protocolList[] = {

{"ControllerTest",24,500,1500,1500,500,1, {button,undefined,undefined,undefined} ,{{ 16,23 },{ -1,-1 },{ -1,-1 },{ -1,-1 }}},
{"Nexus",36,500,1000,500,2000,4,{ batteryStatus,channel,signedTempCelsius,humidityRh },{{8,8},{10,11},{12,23},{28,35}}},
{"Prologue",37,500,2000,500,4000,4,{ batteryStatus,channel,signedTempCelsius,humidityRh },{ {12 , 12},{ 14, 15},{16 ,27 },{28 ,35 } } }

};
static const int nOfProtocols = 3;

int protocolIdentification(int,int);

class GateWay{

    RCSwitch mySwitch;
    RCSwitch mySwitchExtended;
    WiFiServer server =  WiFiServer(defaultPort);
    char* routerName ;
    char* routerPassword;
    protocolMode protocolSet;
    networkMode mode;
    int interruptPin;
    int interruptPinExtended;

    dataSet lastReceivedData;

    void decode();
    void identifyData();
    float setSpecificData(char[],dataType,dataPosition);
    void serialPrintData();
    char* getTypeAsString(dataType);

    public:
    GateWay(int);
    GateWay(int,int);
    GateWay(int,char[],char[]);
    GateWay(int,int,char[],char[]);

    void enable();
    void enable(networkMode);
    bool receive();
    bool sendHtml();

    data getTemperature();
    data getHumidity();
    data getButton();
    data getBatteryStatus();
    data getChannel();





    
    

    
};



#endif