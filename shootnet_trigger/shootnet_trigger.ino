#include <shootnet_trigger.h>

ShootNet_Trigger _trigger;

void setup() 
{
  Serial.begin(115200);
  _trigger.begin();
}

void loop() 
{
  _trigger.iterate(); 
  delay(1); // delay or yield necessary to allow esp8266 to maintain connectivity
}



