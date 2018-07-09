#include <shootnet_sensor.h>


#define PIR_PIN 4  // D2 on wemos board


////////////////////////////////////////
// ShootNet_Sensor::ShootNet_Sensor() //
////////////////////////////////////////
ShootNet_Sensor::ShootNet_Sensor()
: ShootNet_Client()
{}


//////////////////////////////
// ShootNet_Sensor::begin() //
//////////////////////////////
void ShootNet_Sensor::begin()
{
  ShootNet_Client::begin();

  pinMode(PIR_PIN, INPUT);
}

///////////////////////////////////////////
// ShootNet_Sensor::configure_send_msg() //
///////////////////////////////////////////
void ShootNet_Sensor::configure_send_msg()
{
  ShootNet_Unit::configure_send_msg();

  _send_msg.set_unit_type(UNIT_TYPE_SENSOR);
}


////////////////////////////////
// ShootNet_Sensor::iterate() //
////////////////////////////////
void ShootNet_Sensor::iterate()
{
  // update current time and heartbeat
  ShootNet_Unit::iterate();

  // check if any LEDs need to be toggled
  check_blink();

  // Check for incoming messages from controller
  check_for_message();


  if(_armed && HIGH == digitalRead(PIR_PIN))
  {
    //Serial.println("SENSOR DETECT");
    send_message(MSG_ID_SENSOR_DETECT);
    digitalWrite(LED_RED_PIN, HIGH);
    _armed = false; // turns off blinking
  }

  if(_hb_fault_count >= HB_FAULT_LIMIT)
  {
    Serial.print("HB fault count too high; restarting in 2...");
    delay(1000);
    Serial.println("1...");
    delay(1000);
    ESP.restart();
  }
}

////////////////////////////////////////////
// ShootNet_Sensor::handle_rcv_messsage() //
////////////////////////////////////////////
void ShootNet_Sensor::handle_rcv_messsage(IPAddress remote_ip)
{
  switch(_rcv_msg.get_id())
  {
    case MSG_ID_ARM:
      _armed = true;
      break;
    case MSG_ID_RESET:
      _armed = false;
      digitalWrite(LED_RED_PIN, LOW); // in case reset occurs while LED is on
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
    default:
      Serial.printf("  ShootNet_Sensor::handle_rcv_messsage() got invalid message ID: %d\n", _rcv_msg.get_id());
  }
}