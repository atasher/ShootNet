#include <shootnet_trigger.h>

#define RELAY_TRIGGER 4 // D2 on wemos

const unsigned TRIGGER_PERIOD = 2000; // length of time relay is triggered (ms)

//////////////////////////////////////////
// ShootNet_Trigger::ShootNet_Trigger() //
//////////////////////////////////////////
ShootNet_Trigger::ShootNet_Trigger()
: ShootNet_Client(),
  _triggered(false),
  _trigger_time(0)
{}


///////////////////////////////
// ShootNet_Trigger::begin() //
///////////////////////////////
void ShootNet_Trigger::begin()
{
  ShootNet_Client::begin();

  pinMode(RELAY_TRIGGER, OUTPUT);

  digitalWrite(RELAY_TRIGGER, LOW);
}


////////////////////////////////////////////
// ShootNet_Trigger::configure_send_msg() //
////////////////////////////////////////////
void ShootNet_Trigger::configure_send_msg()
{
  ShootNet_Unit::configure_send_msg();

  _send_msg.set_unit_type(UNIT_TYPE_TRIGGER);
}


////////////////////////////////
// ShootNet_Trigger::iterate() //
////////////////////////////////
void ShootNet_Trigger::iterate()
{
  ShootNet_Unit::iterate(); // update timers with current time

  // check if any LEDs need to be toggled
  check_blink();

  // Check for incoming messages from controller
  check_for_message();

  // check if this unit was triggered
  check_trigger();

  if(_hb_fault_count >= HB_FAULT_LIMIT)
  {
    Serial.print("HB fault count too high; restarting in 2...");
    delay(1000);
    Serial.println("1...");
    delay(1000);
    ESP.restart();
  }
}


/////////////////////////////////////////////
// ShootNet_Trigger::handle_rcv_messsage() //
/////////////////////////////////////////////
void ShootNet_Trigger::handle_rcv_messsage(IPAddress remote_ip)
{
  switch(_rcv_msg.get_id())
  {
    case MSG_ID_ARM:
      _armed = true;
      break;
    case MSG_ID_RESET:
      _armed = false;
      digitalWrite(RELAY_TRIGGER, LOW);
      digitalWrite(LED_RED_PIN, LOW);
      break;
    case MSG_ID_LOCATE_ON:
      _locating = true;
      break;
    case MSG_ID_LOCATE_OFF:
      _locating = false;
      // since the blue LED is shared with the locate and connection functions, this
      //  will turn it back on in case locating is turned off while LED is off
      check_connection();
      break;
    case MSG_ID_TRIGGER_FIRE:
      if(_armed)
      {
        _armed = false;
        _triggered = true;
        _trigger_time = millis();
        digitalWrite(RELAY_TRIGGER, HIGH);
        digitalWrite(LED_RED_PIN, HIGH);
      }
      break;
    default:
      Serial.printf("  ShootNet_Trigger::handle_rcv_messsage() got invalid message ID: %d\n", _rcv_msg.get_id());
  }
}

///////////////////////////////////////
// ShootNet_Trigger::check_trigger() //
///////////////////////////////////////
void ShootNet_Trigger::check_trigger()
{
  // only turn the relay on for a set period of time
  if(_triggered)
  {
    if(_t_now >= (_trigger_time+TRIGGER_PERIOD))
    {
      digitalWrite(RELAY_TRIGGER, LOW);
      _triggered = false;
    }
  }
}