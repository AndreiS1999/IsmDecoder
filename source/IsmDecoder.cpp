
/* IsmDecoder: library for ESP8266 (for now testet only on WeMos D1 R1) which implements 
   an gateway between and 433MHz radio devices and cloud.

   This library uses in its implementation some functions from a modified version of 
   rc-swich library ( https://github.com/sui77/rc-switch/ )

*/



#include "IsmDecoder.h"


GateWay::GateWay(int interruptPin,int changeModePin,int resetPinParam ){

    this->mySwitch = RCSwitch();
    this->interruptPin=interruptPin;
    resetPin=resetPinParam;
    this->changeModePin=changeModePin;
    
    this->mode= online;
    this->protocolSet = basic;
    
    
    this->routerName=routerName;
    this->routerPassword=routerPassword;
    this->lastReceivedData.newData=false;
    
     
}
GateWay::GateWay(int interruptPin,int interruptPinExtended,int changeModePin,int resetPinParam ){
    
    this->mySwitch = RCSwitch();
    this->mySwitchExtended = RCSwitch();
    this->interruptPin=interruptPin;
    this->interruptPinExtended=interruptPinExtended;
    resetPin=resetPinParam;
    this->changeModePin=changeModePin;
    
    this->mode= online;
    
    
    this->protocolSet = extended;
  
    this->routerName=routerName;
    this->routerPassword=routerPassword;
    this->lastReceivedData.newData=false;
    
     
     

}


//Gatewy start-up
void GateWay::enable(){
    digitalWrite(resetPin, HIGH);  
    delay(200);
    pinMode(resetPin, OUTPUT); 
    EEPROM.begin(512);
    attachInterrupt(digitalPinToInterrupt(this->changeModePin), swapWorkingModes, FALLING);
    if(EEPROM.read(0x00)!=1){
        wMode=config;
    }
   
   
    
    //setup for the normal working mode
    if(wMode==normal){
        
        
        
        Serial.println(" ");
        Serial.println(" ");
        Serial.println(" ");
        Serial.println(" ");
        Serial.println(" ");
        Serial.println(" ");
        
        
        this->host = "api.thingspeak.com";
        this->applyConfig();
        delay(100);
        this->mode=online;

        
        WiFi.mode(WIFI_STA);  
        WiFi.begin(this->routerName,this->routerPassword );


        Serial.print("Connecting to: ");
        Serial.println(this->routerName);
    
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(500);
        }
        this->server.begin();
        Serial.println(" ");
        Serial.println("Connection succesful...");


        this->mySwitch.enableReceive(this->interruptPin);
        if(this->protocolSet==extended)
            this->mySwitchExtended.enableReceiveExtended(this->interruptPinExtended);
    }

    //setup for the config mode
    else{
        Serial.println("Config mode eanbled...");
        Serial.println();
       
  
        WiFi.softAP(configServerSsid, configServerPassword);

        IPAddress espIP = WiFi.softAPIP();
        Serial.print("IP address: ");
        Serial.println(espIP);
  
        this->server.begin();
        Serial.println("Configuration Web Server ready...");
        Serial.println();
    }
   
  
}


//extracts configurated data from EEPROM and load it to data memory
void GateWay::applyConfig(){
    int addr=1;
    String buffer;
    
    int auxInteger;
    //router ssid
    auxInteger=EEPROM.read(addr);
    addr++;
    for(int i=0;i<auxInteger;i++){
        buffer=buffer+char(EEPROM.read(addr));
        addr++;
        
        
    }
    this->routerName=buffer;
    buffer="";
    
    //router password
    auxInteger=EEPROM.read(addr);
    addr++;
    for(int i=0;i<auxInteger;i++){
        buffer=buffer+char(EEPROM.read(addr));
        addr++;
        
    }
    this->routerPassword=buffer;
    buffer="";

    //thingSpeak channels
    auxInteger=EEPROM.read(addr);
    addr++;
    this->nrOfTsChannels=auxInteger;
    this->tsChannels=new tsChannel[this->nrOfTsChannels];
    for(int j=0;j<this->nrOfTsChannels;j++){
        for(int i=0;i<16;i++){
            buffer=buffer+char(EEPROM.read(addr));
            addr++;
        }
        this->tsChannels[j].channelSendApi=buffer;
        this->tsChannels[j].nrOfFields=0;
        this->tsChannels[j].currentIndex=0;
        buffer="";
    }
    int nrOfFields=EEPROM.read(addr);
    addr++;

    int tempAddr=addr;
    for(int i=0;i<nrOfFields;i++){
        auxInteger=EEPROM.read(addr); //channel index
        addr++;
        this->tsChannels[auxInteger].nrOfFields++;
        auxInteger=EEPROM.read(addr);
        addr=addr+auxInteger+4; //jump to next field

    }
    for(int i=0;i<this->nrOfTsChannels;i++){
        this->tsChannels[i].fields=new tsField[this->tsChannels[i].nrOfFields];
    }
    addr=tempAddr;
    int channelIndex;
    for(int i=0;i<nrOfFields;i++){
        channelIndex=EEPROM.read(addr); 
        addr++;

        auxInteger=EEPROM.read(addr); 
        addr++;
        for(int j=0;j<auxInteger;j++){
            buffer=buffer+char(EEPROM.read(addr));
            addr++;
        }
        this->tsChannels[channelIndex].fields[this->tsChannels[channelIndex].currentIndex].fieldName=buffer;
        buffer="";
        this->tsChannels[channelIndex].fields[this->tsChannels[channelIndex].currentIndex].protocolIndex=EEPROM.read(addr);
        addr++;
        this->tsChannels[channelIndex].fields[this->tsChannels[channelIndex].currentIndex].channel=EEPROM.read(addr)+1;
        addr++;
        this->tsChannels[channelIndex].fields[this->tsChannels[channelIndex].currentIndex].dataTypeIndex=this->intToDataType(EEPROM.read(addr));
        addr++;

        this->tsChannels[channelIndex].currentIndex++;
    }

}


//receives the OOk signal
bool GateWay::receive(){
    if(wMode==normal){
        this->lastReceivedData.newData=false;
        if (mySwitch.available()){


            for(int i=0;i< (mySwitch.getReceivedBitlength()*2+1);i++){
                OOK_data[i]=*(mySwitch.getReceivedRawdata()+i);
                
            }
        

            int protocolNumber=protocolIdentification(mySwitch.getReceivedBitlength(),mySwitch.getReceivedProtocol());
            
            if(protocolNumber!=-1){
                this->lastReceivedData.newData=true;
                this->lastReceivedData.protocolNumber=protocolNumber;
                this->decode();
                this->identifyData();
                this->serialPrintData();
                
        


            }
            mySwitch.resetAvailable();

            
        
            

        }

        if (this->lastReceivedData.newData)
            return true;
        else
            return false;
    }
    else{
        return false;
    }
    

}

//Checks if the number of bits received maches the number defined by the protocol
int protocolIdentification(int bitLength,int protocolNumber){
    int result=-1;
    for(int i=1;i<=nOfProtocols;i++){
        if(bitLength==protocolList[i-1].bitLength && protocolNumber==i){
            result=i;
            break;
        }
    }
    return result;
}


//Converts the OOK micro seconds timings vector to binary code acording to the identified protocol
void GateWay::decode(){
    int j=0;
    for(int i=1; i<=(protocolList[this->lastReceivedData.protocolNumber-1].bitLength)*2; i+=2){
        if(OOK_data[i]>(protocolList[this->lastReceivedData.protocolNumber-1].zeroHighTime-timeTollerance) && OOK_data[i]<(protocolList[lastReceivedData.protocolNumber-1].zeroHighTime+timeTollerance) && OOK_data[i+1]>(protocolList[this->lastReceivedData.protocolNumber-1].zeroLowTime-timeTollerance) && OOK_data[i+1]<(protocolList[this->lastReceivedData.protocolNumber-1].zeroLowTime+timeTollerance)){
            bitData[j]=0;


            
            }
        if(OOK_data[i]>(protocolList[this->lastReceivedData.protocolNumber-1].oneHighTime-timeTollerance) && OOK_data[i]<(protocolList[lastReceivedData.protocolNumber-1].oneHighTime+timeTollerance) && OOK_data[i+1]>(protocolList[this->lastReceivedData.protocolNumber-1].oneLowTime-timeTollerance) && OOK_data[i+1]<(protocolList[this->lastReceivedData.protocolNumber-1].oneLowTime+timeTollerance)){
            bitData[j]=1;
            
            }

        j++;
    }

}


//Corelates each section of the binary code with its signification and applyes the conversion to float data
void GateWay::identifyData(){

    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
        
        this->lastReceivedData.dataVector[j].value=this->setSpecificData(protocolList[this->lastReceivedData.protocolNumber-1].name,protocolList[this->lastReceivedData.protocolNumber-1].dataTypes[j],protocolList[this->lastReceivedData.protocolNumber-1].dataPos[j]);
        this->lastReceivedData.dataVector[j].type=protocolList[this->lastReceivedData.protocolNumber-1].dataTypes[j];
      

    }
   

}

//Converts each section of the received binary code to float data acording to the identifyed protocol
float GateWay::setSpecificData(char protocolName[],dataType type,dataPosition position){
    signed int value=0;
    float finalValue=0;
    if(type==signedTempCelsius){ //signed types
        for(int i=position.startBit+1;i<=position.stopBit;i++){
            value+=bitData[i]*pow(2,position.stopBit-i);
            

        }
        
        if(bitData[position.startBit]==1){
            value=pow(2,position.stopBit-position.startBit)-value;
        }
        
       
    }
    else{
        for(int i=position.startBit;i<=position.stopBit;i++){
            value+=bitData[i]*pow(2,position.stopBit-i);
            

        }
    }

    switch(type){
        case signedTempCelsius:{
            
            finalValue=(float)(value/10);
            
            break;
        };
        case channel:{
            
            finalValue=(float)(value+1);
            
            break;
        };
        default:
            finalValue=(float)value;
        
    }



    return finalValue;   



}


/*
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Print methods:
*/


//Prints last received data set on serial monitor
void GateWay::serialPrintData(){
    
    Serial.print("Received protocol:");
    Serial.print(protocolList[this->lastReceivedData.protocolNumber-1].name);
    Serial.print("   ");
    
    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
        Serial.print(getTypeAsString(lastReceivedData.dataVector[j].type));
        Serial.print(":");
        Serial.print(lastReceivedData.dataVector[j].value);
        Serial.print("   ");
        
        
    }
    Serial.println(" ");
    Serial.println(" ");

}




//checks if the last received data set is compatible with the field's settings given as parameter.
bool GateWay::checkIfValidData(int ch,int f){
    if(tsChannels[ch].fields[f].protocolIndex!=this->lastReceivedData.protocolNumber-1){
        return false;
        Serial.println("protocol prost");
    }
    if(tsChannels[ch].fields[f].channel!=(int)getData(channel)&&(int)getData(channel)!=-999){
        return false;
        Serial.println("canal prost");
    }
    if((int)getData(tsChannels[ch].fields[f].dataTypeIndex)==-999){
        return false;
        Serial.println("tip_data prost");
    }

    return true;

}


//send request to thingspeak if some data available.
bool GateWay::sendToThingSpeak(){
   if(wMode==normal && this->lastReceivedData.newData==true){
        WiFiClient client;          
        const int httpPort = 80; 
        
        String link="";
        for(int i=0;i<this->nrOfTsChannels;i++){
            if(!client.connect(this->host, httpPort)){
                Serial.println("Connection Failed");
                delay(300);
                return false; 
            }
            link=link+"GET /update?api_key="+this->tsChannels[i].channelSendApi;
            delay(100);
            bool emptyLink=true;
            for(int j=0;j<this->tsChannels[i].nrOfFields;j++){
                if(checkIfValidData(i,j)){
                    emptyLink=false;
                    link=link+"&"+tsChannels[i].fields[j].fieldName+"="+String(getData(tsChannels[i].fields[j].dataTypeIndex));
                }

            }
            if(emptyLink==false){
                link = link + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";  
                Serial.println(link);

                      
                client.print(link);
                delay(100);
                int timeout=0;
                while((!client.available()) && (timeout < 3000)) {   
                delay(10);  
                    timeout++;
                }
                if(timeout < 500){
                    while(client.available()){
                        Serial.println(client.readString());    
                    }
                }
                else{
                    Serial.println("Request timeout..");
                }

                client.stop();

            
                delay(100);
            }
            link="";
            
        }
        
        
    }
   else{
       return false;
   }

}

data GateWay::getTemperature(){

    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
       if(lastReceivedData.dataVector[j].type==signedTempCelsius){
           return {signedTempCelsius,lastReceivedData.dataVector[j].value};
       }
        
        
    }

    return {undefined,-999};
}
data GateWay::getHumidity(){


    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
       if(lastReceivedData.dataVector[j].type==humidityRh){
           return {humidityRh,lastReceivedData.dataVector[j].value};
       }
        
        
    }

    return {undefined,-999};

}
data GateWay::getButton(){

    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
       if(lastReceivedData.dataVector[j].type==button){
           return {button,lastReceivedData.dataVector[j].value};
       }
        
        
    }

    return {undefined,-999};

}
data GateWay::getBatteryStatus(){

    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
       if(lastReceivedData.dataVector[j].type==batteryStatus){
           return {batteryStatus,lastReceivedData.dataVector[j].value};
       }
        
        
    }

    return {undefined,-999};

}
data GateWay::getChannel(){

    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
       if(lastReceivedData.dataVector[j].type==channel){
           return {channel,lastReceivedData.dataVector[j].value};
       }
        
        
    }
    
    return {undefined,-999};

}

float GateWay::getData(dataType type){
    for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){
       if(lastReceivedData.dataVector[j].type==type){
           return lastReceivedData.dataVector[j].value;
       }
        
        
    }
    
    return -999;
}


//change working mode (normal to config or config to normal)
void swapWorkingModes(){
  if(wMode==normal){
    wMode=config;
    EEPROM.write(0, 0);
    EEPROM.commit();
    
  }

  else
    wMode=normal;

  delay(500);
  digitalWrite(resetPin, LOW);
}


//save data form the config web formular
void GateWay::handleConfig(){
    if(wMode==config){
        delay(1000);
    
        WiFiClient client = server.available();
        if (!client) {

            return;
        }   
    
        
        String request = client.readStringUntil('\r');

        client.flush();
        String buffer="";
        int addr=1;
    
        if(request.indexOf("&pass")!=-1){

                
                //saving routers ssid--------------------------------------------------------
                int start_index=request.indexOf("ssid=")+5;
                int stop_index=request.indexOf("&pass");
        
                buffer=request.substring(start_index,stop_index);
                Serial.println(buffer);
                EEPROM.write(addr, buffer.length());
                addr++;
                for(int i=0;i<buffer.length();i++){
                    EEPROM.write(addr, buffer[i]);
                    addr++;
                }
                buffer="";
                //----------------------------------------------------------------------------



                
                //saving router password-------------------------------------------------------
                start_index=request.indexOf("pass=")+5;
                stop_index=request.indexOf("&nrOfFields");
        
                

                buffer=request.substring(start_index,stop_index);
                EEPROM.write(addr, buffer.length());
                Serial.println(buffer);
                addr++;
                for(int i=0;i<buffer.length();i++){
                    EEPROM.write(addr, buffer[i]);
                    addr++;
                }
                buffer="";
                //--------------------------------------------------------------------------------


              
                //saving channels api & nr of channels-------------------------------------------------------------
                int j=1;
                int nrOfChannels=0;
                String channels="";
                while(true){
                    start_index=request.indexOf("api"+String(j)+"=")+5;
                    stop_index=request.indexOf("&prot"+String(j));

                    if(start_index!=-1 && stop_index!=-1){
                        j++;

                        buffer=request.substring(start_index,stop_index);
                        if(channels.indexOf(buffer)==-1){
                            Serial.println(buffer);
                            channels=channels+buffer;
                            nrOfChannels++;
                        }
                        buffer="";
                    }
                    else{
                        
                        break;
                    }
                        
                }


                Serial.println(nrOfChannels);
                EEPROM.write(addr, nrOfChannels);// nr of channels
                addr++;
                Serial.println(channels);
                for(int i=0;i<channels.length();i++){
                    EEPROM.write(addr, channels[i]);
                    addr++;
                }
                


                //saving number of fields --------------------------------------------------------
                start_index=request.indexOf("nrOfFields=")+11;
                stop_index=request.indexOf("&fldName1");
                
                buffer=buffer=request.substring(start_index,stop_index);
                int nrOfFields=buffer.toInt();
                EEPROM.write(addr, nrOfFields);
                addr++;
                Serial.println(nrOfFields);
                buffer="";
                //--------------------------------------------------------------------------------

                

                //saving fields data--------------------------------------------------------------
                
                int nrAux;
                for(int i=1;i<=nrOfFields;i++){

                    //field channel index
                    start_index=request.indexOf("api"+String(i)+"=")+5;
                    stop_index=request.indexOf("&prot"+String(i));
                    buffer=request.substring(start_index,stop_index);

                    nrAux=channels.indexOf(buffer)/16;
                    EEPROM.write(addr, nrAux);
                    Serial.println(nrAux);
                    addr++;
                    

                    
                    //field name:
                    
                    start_index=request.indexOf("fldName"+String(i)+"=")+9;
                    stop_index=request.indexOf("&api"+String(i));

                    buffer=request.substring(start_index,stop_index);
                    EEPROM.write(addr, buffer.length());
                    Serial.println(buffer);
                    addr++;
                    for(int i=0;i<buffer.length();i++){
                        EEPROM.write(addr, buffer[i]);
                        addr++;
                    }
                    buffer="";

                    //protocol index:

                    start_index=request.indexOf("prot"+String(i)+"=")+6;
                    stop_index=request.indexOf("&ch"+String(i));

                    buffer=request.substring(start_index,stop_index);

                    nrAux=buffer.toInt();
                    EEPROM.write(addr, nrAux);
                    Serial.println(nrAux);
                    addr++;
                    buffer="";

                    //ch index:

                    start_index=request.indexOf("ch"+String(i)+"=")+4;
                    stop_index=request.indexOf("&dt"+String(i));

                    buffer=request.substring(start_index,stop_index);

                    nrAux=buffer.toInt();
                    EEPROM.write(addr, nrAux);
                    Serial.println(nrAux);
                    addr++;
                    buffer="";

                    //data_type index:

                    start_index=request.indexOf("dt"+String(i)+"=")+4;
                    if(i==nrOfFields){
                       stop_index=request.indexOf(" HTTP/1."); 
                    }
                    else{
                        stop_index=request.indexOf("&fldName"+String(i+1));
                    }
                    

                    buffer=request.substring(start_index,stop_index);

                    nrAux=buffer.toInt();
                    EEPROM.write(addr, nrAux);
                    Serial.println(nrAux);
                    addr++;
                    buffer="";
                  
                    
                }
                //-------------------------------------------------------------------------------


                

                EEPROM.write(0, 1);
                

                EEPROM.commit();
                delay(1000);

                swapWorkingModes();



                
            
        }
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); 
        client.println(form);
    
        delay(1);
        
    }
}

//int to dataType data type conversion
dataType GateWay::intToDataType(int type){
    switch(type){
        case 0:{

            return undefined;
            break;
        };
        case 1:{

            return signedTempCelsius;
            break;
        };
        case 2:{

            return humidityRh;
            break;
        };
        case 3:{

            return batteryStatus;
            break;
        };
        case 4:{

            return channel;
            break;
        };
        case 5:{

            return button;
            break;
        };
        default:
            return undefined;
        
    }
}

//Converion form 'dataType' to 'char*'
char* GateWay::getTypeAsString(dataType type){
    switch(type){
        case signedTempCelsius:{
            
            return "signedTempCelsius";
            
            break;
        };
          case batteryStatus:{
            
            return "batteryStatus";
            
            break;
        };
          case humidityRh:{
            
            return "humidityRh";
            
            break;
        };
          case channel:{
            
            return "channel";
            
            break;
        };
          case button:{
            
            return "button";
            
            break;
        };
          case undefined:{
            
            return "undefined";
            
            break;
        };
        
        default:
            return "unknown";
        
    }

}




