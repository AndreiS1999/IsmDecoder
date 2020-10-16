#include <IsmDecoder.h>

GateWay myGateWay(D8,D12,"router_name","router_password");

void setup() {
 
 Serial.begin(115200);
 myGateWay.enable();

}

void loop() {
 
  
  myGateWay.receive();
  myGateWay.sendHtml();
     
 
  delay(1);
  
 
}
