#include <shootnet_controller.h>

ShootNet_Controller _controller;


void setup() 
{
  Serial.begin(115200);
  _controller.begin();  
}


////////////
// loop() //
////////////
void loop() 
{
  _controller.iterate();
  delay(1); // delay or yield necessary to allow esp8266 to maintain connectivity
}






