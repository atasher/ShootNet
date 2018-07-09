#include <shootnet_controller.h>

#define BUTTON_UP  12  // D6 on wemos board
#define BUTTON_DN  13  // D7 on wemos board
#define BUTTON_SEL 2  // D8 on wemos board






////////////////////////////////////////////////
// ShootNet_Controller::ShootNet_Controller() //
////////////////////////////////////////////////
ShootNet_Controller::ShootNet_Controller()
: ShootNet_Unit(),
  _menu(*this),
  _button_up(BUTTON_UP),
  _button_dn(BUTTON_DN),
  _button_sel(BUTTON_SEL)
{
}


//////////////////////////////////
// ShootNet_Controller::begin() //
//////////////////////////////////
void ShootNet_Controller::begin()
{
  _menu.setup();

  ShootNet_Unit::begin();

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DN, INPUT_PULLUP);
  pinMode(BUTTON_SEL, INPUT_PULLUP);
}


////////////////////////////////////
// ShootNet_Controller::iterate() //
////////////////////////////////////
void ShootNet_Controller::iterate()
{
  ShootNet_Unit::iterate();

  // Check for incoming messages from clients
  check_for_message();

  // Check if any units need to be triggered
  check_triggers();

  if(_button_up.pressed())
  {
    _menu.up();
  }
  if(_button_dn.pressed())
  {
    _menu.down();
  }
  if(_button_sel.pressed())
  {
    _menu.select();
  }
}


///////////////////////////////////////////////
// ShootNet_Controller::configure_send_msg() //
///////////////////////////////////////////////
void ShootNet_Controller::configure_send_msg()
{
  ShootNet_Unit::configure_send_msg();

  _send_msg.set_unit_type(UNIT_TYPE_CONTROLLER);
}


////////////////////////////////////////////////
// ShootNet_Controller::handle_rcv_messsage() //
////////////////////////////////////////////////
void ShootNet_Controller::handle_rcv_messsage(IPAddress remote_ip)
{
  switch(_rcv_msg.get_id())
  {
    case MSG_ID_HEARTBEAT:
      _client_units.register_unit(_rcv_msg, remote_ip);
      break;
    case MSG_ID_SENSOR_DETECT:
      _client_units.trigger(mac_to_string(_rcv_msg.get_mac()));
      break;
    default:
      Serial.printf("  ShootNet_Controller::handle_rcv_messsage() got invalid message ID: %d\n", _rcv_msg.get_id());
  }
}


//////////////////////////////////////
// ShootNet_Controller::run_stage() //
//////////////////////////////////////
void ShootNet_Controller::run_stage(unsigned idx)
{
  unsigned num_units;
  IPAddress unit_ips[MAX_UNITS];

  if(_client_units.get_stage_ips(idx, num_units, unit_ips))
  {
    for(int i=0; i<num_units; ++i)
    {
      send_message(MSG_ID_ARM, unit_ips[i]);
    }
  }
}


////////////////////////////////////////
// ShootNet_Controller::reset_stage() //
////////////////////////////////////////
void ShootNet_Controller::reset_stage(unsigned idx)
{
  unsigned num_units;
  IPAddress unit_ips[MAX_UNITS];

  if(_client_units.get_stage_ips(idx, num_units, unit_ips))
  {
    for(int i=0; i<num_units; ++i)
    {
      send_message(MSG_ID_RESET, unit_ips[i]);
    }
  }
}


////////////////////////////////////////
// ShootNet_Controller::reset_stage() //
////////////////////////////////////////
void ShootNet_Controller::locate(const String& mac)
{
  const IPAddress IP_ZERO(0,0,0,0);

  IPAddress unit_ip;

  // check if this is a valid mac
  if(_client_units.get_ip_from_mac(mac, unit_ip))
  {
    // if so, check if the unit is the same as the currently locating unit
    if(_locating_ip != unit_ip)
    {
      // make sure it's a valid IP; turn off locating for the old IP
      if(_locating_ip != IP_ZERO)
      {
        send_message(MSG_ID_LOCATE_OFF, _locating_ip);
      }

      // start locating the new current IP
      _locating_ip = unit_ip;
      send_message(MSG_ID_LOCATE_ON, _locating_ip);
    }
  }
  else
  {
    // if it was an invalid mac, turn off locating for the previously selected
    //  unit, if there was one
    if(_locating_ip != IP_ZERO)
    {
      send_message(MSG_ID_LOCATE_OFF, _locating_ip);
      _locating_ip = IP_ZERO;
    }
  }
}


///////////////////////////////////////////
// ShootNet_Controller::check_triggers() //
///////////////////////////////////////////
void ShootNet_Controller::check_triggers()
{
  unsigned num_trigs;
  IPAddress trig_ips[MAX_UNITS];

  // check each unit to see if its trigger time was set to a non-zero value (meaning it was triggered)
  for(int i=0; i<_client_units._num_sensors; ++i)
  {
    if(_client_units._sensor_map[i].trigger_time > 0)
    {
      if(millis() >= _client_units._sensor_map[i].trigger_time + _client_units._sensor_map[i].trigger_delay)
      {
        // 0 is used to indicate not-triggered as it is an (essentially) impossible trigger time
        _client_units._sensor_map[i].trigger_time = 0;

        // look for assigned triggers and send fire cmd
        if(_client_units.get_mapped_triggers(_client_units._sensor_map[i].sensor_mac, num_trigs, trig_ips))
        {
          for(int i=0; i<num_trigs; ++i)
          {
            send_message(MSG_ID_TRIGGER_FIRE, trig_ips[i]);
          }
        }
      }
    }
  }
}