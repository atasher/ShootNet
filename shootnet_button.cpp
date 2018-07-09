#include <shootnet_button.h>
#include <Arduino.h>

const unsigned BUTTON_THRESH = 25; // ms


////////////////////////////////////////
// ShootNet_Button::ShootNet_Button() //
////////////////////////////////////////
ShootNet_Button::ShootNet_Button(int pin)
{
  _pin = pin;
  _t_bpress = 0;
  _b_state_prev = HIGH;
  _b_state = HIGH;
}

////////////////////////////////
// ShootNet_Button::pressed() //
////////////////////////////////
bool ShootNet_Button::pressed()
{
  bool pressed = false;

  unsigned tnow = millis();

  int b_read = digitalRead(_pin); // button read value
  //Serial.printf("b_read=%d\n", b_read);
  if(b_read != _b_state_prev)
  {
    _t_bpress = tnow;
  }

  if((tnow - _t_bpress) >= BUTTON_THRESH)
  {
    if(b_read != _b_state)
    {
      _b_state = b_read;

      if(_b_state == HIGH)
      {
        pressed = true;
      }
    }
  }
  _b_state_prev = b_read;

  return pressed;
}