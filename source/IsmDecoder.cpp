
/* IsmDecoder: library for ESP8266 (for now testet only on WeMos D1 R1) which implements 
   an gateway between and 433MHz radio devices and cloud.

   This library uses in its implementation some functions from a modified version of 
   rc-swich library ( https://github.com/sui77/rc-switch/ )

*/



#include "IsmDecoder.h"


GateWay::GateWay(int interruptPin){
    
    this->mySwitch = RCSwitch();
    this->interruptPin=interruptPin;
    
    this->mode= offline;
    this->protocolSet = basic;
    this->lastReceivedData.newData=false;
    
    
    

}
GateWay::GateWay(int interruptPin,int interruptPinExtended){
    
    this->mySwitch = RCSwitch();
    this->mySwitchExtended = RCSwitch();
    this->interruptPin=interruptPin;
    this->interruptPinExtended=interruptPinExtended;
    
    this->mode= offline;
    this->protocolSet = extended;
    this->lastReceivedData.newData=false;

}
GateWay::GateWay(int interruptPin,char routerName[] ,char routerPassword[] ){

    this->mySwitch = RCSwitch();
    this->interruptPin=interruptPin;
    
    this->mode= online;
    this->protocolSet = basic;
    
    this->routerName=routerName;
    this->routerPassword=routerPassword;
    this->lastReceivedData.newData=false;
    
     
}
GateWay::GateWay(int interruptPin,int interruptPinExtended,char routerName[] ,char routerPassword[] ){
    
    this->mySwitch = RCSwitch();
    this->mySwitchExtended = RCSwitch();
    this->interruptPin=interruptPin;
    this->interruptPinExtended=interruptPinExtended;
    
    this->mode= online;
    
    this->protocolSet = extended;
  
    this->routerName=routerName;
    this->routerPassword=routerPassword;
    this->lastReceivedData.newData=false;
    
     
     

}


//Gatewy start-up
void GateWay::enable(){
    
    delay(4000);
    this->mode=online;
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
    Serial.print("URL: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");


    this->mySwitch.enableReceive(this->interruptPin);
    if(this->protocolSet==extended)
        this->mySwitchExtended.enableReceiveExtended(this->interruptPinExtended);
   
  
}

void GateWay::enable(networkMode mode){
    if(mode==online)
        this->enable();
    if(mode==offline){
        delay(10);
        this->mySwitch.enableReceive(this->interruptPin);
        if(this->protocolSet==extended)
            this->mySwitchExtended.enableReceiveExtended(this->interruptPinExtended);
    }
}


//receives the OOk signal
bool GateWay::receive(){
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



//Transmit by ESP2866 in HTML format using HTTP to URL recevied when connection was initated
bool GateWay::sendHtml(){

    //if(this->lastReceivedData.newData){
        WiFiClient client = this->server.available();
        if (!client) {
        
        
            return false;

        }
        
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); 
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");

        client.print("Received protocol:<b>");
        client.print(protocolList[this->lastReceivedData.protocolNumber-1].name);
        client.print("</b><br>");

        for(int j=0;j<protocolList[this->lastReceivedData.protocolNumber-1].dataLength;j++){

            client.print(getTypeAsString(lastReceivedData.dataVector[j].type));
            client.print(":<b>");
            client.print(lastReceivedData.dataVector[j].value);
            client.print("</b><br>");



        }
        return true;
    //}

    //else{
       //return false; 
    //}

    

       

    
    

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

