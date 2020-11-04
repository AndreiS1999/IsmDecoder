# ISM DECODER LIBRARY    

This is a library for ESP8266 (for now testet only on WeMos D1 R1) which implements 
a gateway between and 433MHz radio devices and the internet.


This library uses in its implementation some functions from a modified version of 
rc-swich library.

## General Description
The gateway implemented by the library has 2 working modes: NORMAL and CONFIG.
In the CONFIG mode the development board can be connected by ESP directly to any device with WiFi capabilities from where a configuration can be defined.
In the NORMAL mode the develooment board can be connected to the interned using a WiFi Router to send data from 433Hhz devices to the cloud.

## Suported 433MHz Devices
There are two packages of protocols: basic and extended.
### Basic package

#### Home HCKK 08
- Data: temperature, humidity, battery status
- Channels: 3 channels
- Components: integrated circuit S522BR - RF



## Recomanded setup for the simpleGateWay example sketch

### Hardware Requirments

- Development board with ESP8266 module(tested only on WeMos D1 R1)
- RF receive module for 433MHZ
- any switch
- 10k resistor

### Pins configuration

- 1 or 2 interrupt pins for RF module data outpusts.(The second pin is required for the "extended" package of protocols. With only 1 pin the gateway will
work fine, but with only the basic package)
- 1 interrupt pin for the reset config button output.
- 1 pin for the "software" reset required for working modes swap. This pin have to be connected with the development board's reset pin.


### Recomanded Schematic
![schematic](https://github.com/AndreiS1999/IsmDecoder/blob/main/img/simple_schematic.jpg?raw=true)

Basic Protocol Set:
	1.ControllerTest 
	2.Nexus (DG-R8H, Home-HCKK 08)

Extended Protocol Set:
	1.Prologue (DIGOO R8S)

*Prologue is added in the extended set of protocols because of its timings incompatibility with 
nexus.

METHODS

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








