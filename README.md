#####################################################################################################
#################################     ISM DECODER LIBRARY    ########################################
#####################################################################################################

This is a library for ESP8266 (for now testet only on WeMos D1 R1) which implements 
a gateway between and 433MHz radio devices and cloud.

This library uses in its implementation some functions from a modified version of 
rc-swich library.



Hardware requirements
####################################################################################################
- Development board with ESP8266 module(tested only on WeMos D1 R1)
- RF receive module for 433MHZ


Pins
####################################################################################################
1 or 2 interrupt pins for RF module data outpusts.
The second pin is required for the "extended" set of protocols( with only 1 pin the gateway will
work fine, but with only the basic set)

Basic Protocol Set:
	1.ControllerTest 
	2.Nexus (DG-R8H, Home-HCKK 08)

Extended Protocol Set:
	1.Prologue (DIGOO R8S)

*Prologue is added in the extended set of protocols because of its timings incompatibility with 
nexus.


Methods
####################################################################################################

1.Constructors:

GateWay(int pin1, char* router_name, char* router_password);
GateWay(int pin1,int pin2, char* router_name, char* router_password);

2.Methods:

enanle(); - have to be used in setup()
enable(networkMode mode) - enables the gateWay in "online"(conected by ESP) or "offline"( working only 
as receiver and decoder).Default mode, applyed by enable(); is online.

receive(); - checks if the signal is similar to a protocl and saves the recevied data.

sendHtml(); - sends the last received data on the local server as HTML;

getTemperature();
getHumidity();
getButton();
getBatteryStatus();
getChannel();








