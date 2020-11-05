# ISM DECODER LIBRARY    

This is a library for ESP8266 (for now testet only on WeMos D1 R1) which implements 
a gateway between and 433MHz radio devices and the internet.


This library uses in its implementation some functions from a modified version of 
rc-swich library.

## General Description
The gateway implemented by the library has 2 working modes: NORMAL and CONFIG.
In the CONFIG mode the development board can be connected by ESP directly to any device with WiFi capabilities from where a configuration can be defined.
In the NORMAL mode the development board can be connected to the internet using a WiFi Router to send data from 433Hhz devices to the cloud.

## Suported 433MHz Devices
There are two packages of devices: basic and extended.
### Basic package

#### Home HCKK 08
- Data: temperature, humidity, battery status
- Channels: 3 channels
- Components: integrated circuit S522BR - RF
- Protocol: Nexus

<img src="https://github.com/AndreiS1999/IsmDecoder/blob/main/img/homeWhiteOutside.jpg" width="30%" height="30%">
<img src="https://github.com/AndreiS1999/IsmDecoder/blob/main/img/whiteHomeFront2.jpg" width="30%" height="30%">

#### DG - R8H
- Data: temperature, humidity, battery status
- Channels: 3 channels
- Components: integrated circuit XY511M - 1TX
- Protocol: Nexus

<img src="https://github.com/AndreiS1999/IsmDecoder/blob/main/img/DigooR8HOutside.jpg" width="30%" height="30%">
<img src="https://github.com/AndreiS1999/IsmDecoder/blob/main/img/DGR8HFront.jpg" width="30%" height="30%">

### Extended Package

#### DG - R8S
- Data: temperature, humidity, battery status
- Channels: 3 channels
- Components: integrated circuit TX1 -5 , radio emitter GE16 -1077R5
- Protocol: Prologue

<img src="https://github.com/AndreiS1999/IsmDecoder/blob/main/img/DGr8sOutside.jpg" width="30%" height="30%">
<img src="https://github.com/AndreiS1999/IsmDecoder/blob/main/img/DIGOOr8sFront.jpg" width="30%" height="30%">

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



## Methods and Functions

### Constructors:

- GateWay(data pin , reset config pin, output pin for RST pin);
- GateWay(data pin 1 , data pin 2, reset config pin, output pin for RST pin);

### Methods:

- enanle(); - have to be used in setup()
- receive(); - checks if the signal is similar to a protocl and saves the recevied data.
- sendToThingSpeak(); - sends the last received data package to the thinngspeak channel/field acording to teh configuration
- handleConfig(); - used only in config mode, receives data by GET from the device connected to ESP and saves the configuration in the EEPROM mamory


- getData( data type )
- getTemperature();
- getHumidity();
- getButton();
- getBatteryStatus();
- getChannel();








