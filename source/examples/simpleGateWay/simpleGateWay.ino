#include <IsmDecoder.h>

GateWay myGateWay(D8,D12,D13,D2);

/*The gateway has two working modes: NORMAL and CONFIG.
 *If the first address of EEPROM memory is not 1 the setup for CONFIG mode will be executed, else, the setup for NORMAL mode will be executed.
 *You can exit CONFIG mode by appling some setting using the configuration pannel.
 *You can acces the configuration pannel in your web browser at http://192.168.4.1/ if you are connected at GatwayConfig WiFi network.
 *You can reset the configuration with a HIGH signal on the reset_config_pin (in this example: D13) 
 *Recomandation: erase EEPROM first time you compile this sketch
 */


void setup() {
 
 Serial.begin(115200);
 myGateWay.enable(); 
/*required to set some data which cannot be seted in the 
*GateWay constructor due to its ambigous characteristics
*/
 }

void loop() {
 
  
  myGateWay.receive(); //actevates an interrupt and saves received data in the GateWay object
  myGateWay.sendToThingSpeak(); //sends data to ThingSpeak acorting to your configuration.
  myGateWay.handleConfig(); //only in CONFIG mode, used to apply data from the config pannel
     
 
  delay(1);
  
 
}
