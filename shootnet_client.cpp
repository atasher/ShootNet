#include <shootnet_client.h>

// Number of times a heartbeat must fail before client resets itself
const unsigned HB_FAULT_LIMIT = 5;

// blink interval for locate/armed
const unsigned BLINK_INTERVAL = 200; //ms


////////////////////////////////////////
// ShootNet_Client::ShootNet_Client() //
////////////////////////////////////////
ShootNet_Client::ShootNet_Client()
: ShootNet_Unit(),
  _hb_fault_count(0),
  _last_blink(0),
  _armed(false),
  _blink_on(false)
{}


////////////////////////////
// ShootNet_Client::begin //
////////////////////////////
void ShootNet_Client::begin()
{
  ShootNet_Unit::begin();

  // setup client-specific pins
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
}


//////////////////////////////////
// ShootNet_Client::heartbeat() //
//////////////////////////////////
void ShootNet_Client::heartbeat()
{
  ShootNet_Unit::heartbeat();

  // only clients attempt the heartbeat message to the controller
  if(send_message(MSG_ID_HEARTBEAT))
  {
    _hb_fault_count = 0;
    digitalWrite(LED_GREEN_PIN, HIGH);
  }
  else
  {
    _hb_fault_count++;
    digitalWrite(LED_GREEN_PIN, LOW);
  }
}


////////////////////////////////////
// ShootNet_Client::check_blink() //
////////////////////////////////////
void ShootNet_Client::check_blink()
{
  // check if the red or blue LED needs to blink
  if(_armed || _locating)
  {
    if(_t_now >= (_last_blink+BLINK_INTERVAL))
    {
      _last_blink = _t_now;

      if(_blink_on)
      {
        if(_armed)
        {
          digitalWrite(LED_RED_PIN, HIGH);
        }
        if(_locating)
        {
          digitalWrite(LED_BLUE_PIN, HIGH);
        }
      }
      else
      {
        if(_armed)
        {
          digitalWrite(LED_RED_PIN, LOW);
        }
        if(_locating)
        {
          digitalWrite(LED_BLUE_PIN, LOW);
        }
      }

      _blink_on = !_blink_on;
    }
  }
}