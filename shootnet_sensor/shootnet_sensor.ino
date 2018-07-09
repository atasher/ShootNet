#include <shootnet_sensor.h>

ShootNet_Sensor _sensor; 

void setup() 
{
  Serial.begin(115200);
  _sensor.begin();
}


void loop() 
{
  _sensor.iterate(); 
  delay(1); // delay or yield necessary to allow esp8266 to maintain connectivity
}



