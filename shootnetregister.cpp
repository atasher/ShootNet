#include <Arduino.h>
#include <shootnetregister.h>
#include <shootnetmsg.h>

////////////////////////////////////////
// ShootNetRegister::ShootNetRegister //
////////////////////////////////////////
ShootNetRegister::ShootNetRegister()
{
  _num_units = 0;
  _num_sensors = 0;

  for(int i=0; i<MAX_UNITS; ++i)
  {
    _register_map[i].mac = "";
    _register_map[i].ip = {0,0,0,0};
    _register_map[i].unit_type = (UnitType)0;

    _sensor_map[i].trigger_time = 0;
    _sensor_map[i].trigger_delay = 0;
    _sensor_map[i].sensor_mac = "";
    _sensor_map[i].num_triggers = 0;
    for(int j=0; j<MAX_UNITS; ++j)
    {
      _sensor_map[i].trigger_macs[j] = "";
    }
  }

  for(int i=0; i<MAX_STAGES; ++i)
  {
    _stage_map[i].num_sensors = 0;
    for(int j=0; j<MAX_UNITS; ++j)
    {
      _stage_map[i].sensor_macs[j] = "";
    }
  }
}


/////////////////////////////////////
// ShootNetRegister::register_unit //
/////////////////////////////////////
void ShootNetRegister::register_unit(ShootNetMsg& msg, IPAddress ip)
{
  String new_mac = mac_to_string(msg.get_mac());

  bool is_new_unit = true;
  for(int i=0; i<_num_units; ++i)
  {
    if(_register_map[i].mac.equals(new_mac))
    {
      is_new_unit = false;

      _register_map[i].ip = ip;
      _register_map[i].unit_type = msg.get_unit_type();

      break;
    }
  }

  if(is_new_unit && (_num_units < MAX_UNITS))
  {
    unsigned new_index = _num_units;
    _num_units++;

    _register_map[new_index].mac = new_mac;
    _register_map[new_index].ip = ip;
    _register_map[new_index].unit_type = msg.get_unit_type();

    if(UNIT_TYPE_SENSOR == _register_map[new_index].unit_type)
    {
      _sensor_map[_num_sensors].sensor_mac = new_mac;
      _num_sensors++;
    }

    print();
  }
}


////////////////////////////////////////
// ShootNetRegister::map_stage_sensor //
////////////////////////////////////////
bool ShootNetRegister::map_stage_sensor(unsigned stage_idx, const String& sensor_mac)
{
  bool select_sensor = true;

  if(stage_idx >= MAX_STAGES)
  {
    Serial.printf("ERROR: stage index %d too large\n", stage_idx);
    return false;
  }

  unsigned& num_sensors = _stage_map[stage_idx].num_sensors;
  String*   sensor_macs = _stage_map[stage_idx].sensor_macs;

  for(int i=0; i<num_sensors; ++i)
  {
    if(sensor_macs[i].equals(sensor_mac))
    {
      select_sensor = false;
      // remove from _stage_map
      for(int j=i; j<num_sensors-1; ++j)
      {
        sensor_macs[j] = sensor_macs[j+1];
      }
      num_sensors--;
      break;
    }
  }

  if(select_sensor && num_sensors < MAX_UNITS)
  {
    sensor_macs[num_sensors] = sensor_mac;
    num_sensors++;
  }
  else
  {
    select_sensor = false; // in case num_sensors is too big
  }

  return select_sensor;
}


//////////////////////////////////////////
// ShootNetRegister::map_sensor_trigger //
//////////////////////////////////////////
bool ShootNetRegister::map_sensor_trigger(const String& sensor_mac, const String& trigger_mac)
{
  bool select_trigger = true;

  int map_idx = -1;
  for(int i=0; i<MAX_UNITS; ++i)
  {
    if(sensor_mac.equals(_sensor_map[i].sensor_mac))
    {
      map_idx = i;
      break;
    }
  }

  if(map_idx < 0)
  {
    Serial.printf("ERROR: sensor mac %s not found in sensor map\n", sensor_mac.c_str());
    return false;
  }

  unsigned& num_triggers = _sensor_map[map_idx].num_triggers;
  String*  triggers_macs = _sensor_map[map_idx].trigger_macs;

  for(int i=0; i<num_triggers; ++i)
  {
    if(triggers_macs[i].equals(trigger_mac))
    {
      select_trigger = false;
      // remove from _sensor_map
      for(int j=i; j<num_triggers-1; ++j)
      {
        triggers_macs[j] = triggers_macs[j+1];
      }
      num_triggers--;
      break;
    }
  }

  if(select_trigger && num_triggers < MAX_UNITS)
  {
    triggers_macs[num_triggers] = trigger_mac;
    num_triggers++;
  }
  else
  {
    select_trigger = false; // in case num_sensors is too big
  }

  return select_trigger;
}


////////////////////////////////////////////////
// ShootNetRegister::set_stage_map_selections //
////////////////////////////////////////////////
void ShootNetRegister::set_stage_map_selections(unsigned stage_idx, String* macs, unsigned num_lines, bool* selections)
{
  unsigned& num_sensors = _stage_map[stage_idx].num_sensors;
  String*   sensor_macs = _stage_map[stage_idx].sensor_macs;

  for(int i=0; i<num_sensors; ++i)
  {
    for(int j=0; j<num_lines; ++j)
    {
      selections[i] = false;
      if(sensor_macs[i].equals(macs[j]))
      {
        selections[j] = true;
        break;
      }
    }
  }
}


////////////////////////////////////////////////
// ShootNetRegister::set_sensor_map_selections //
////////////////////////////////////////////////
void ShootNetRegister::set_sensor_map_selections(const String& sensor_mac, String* trigger_macs, unsigned num_lines, bool* selections)
{
  int map_idx = -1;
  for(int i=0; i<MAX_UNITS; ++i)
  {
    if(sensor_mac.equals(_sensor_map[i].sensor_mac))
    {
      map_idx = i;
      break;
    }
  }

  if(map_idx < 0)
  {
    Serial.printf("ERROR: sensor mac %s not found in sensor map\n", sensor_mac.c_str());
    return;
  }

  unsigned& num_triggers = _sensor_map[map_idx].num_triggers;
  String*   map_triggers = _sensor_map[map_idx].trigger_macs;

  for(int i=0; i<num_triggers; ++i)
  {
    for(int j=0; j<num_lines; ++j)
    {
      selections[i] = false;
      if(trigger_macs[i].equals(map_triggers[j]))
      {
        selections[j] = true;
        break;
      }
    }
  }
}


/////////////////////////////////////
// ShootNetRegister::get_stage_ips //
/////////////////////////////////////
bool ShootNetRegister::get_stage_ips(unsigned stage_idx, unsigned& num_units, IPAddress* unit_ips)
{
  if(stage_idx >= MAX_STAGES)
  {
    Serial.printf("ERROR: stage index %d too large\n", stage_idx);
    return false;
  }

  num_units = 0;
  String* sensor_macs = _stage_map[stage_idx].sensor_macs;

  for(int i=0; i<_stage_map[stage_idx].num_sensors; ++i)
  {
    unsigned num_trigs;
    IPAddress trig_ips[MAX_UNITS];

    // get triggers mapped to this sensor
    get_mapped_triggers(sensor_macs[i], num_trigs, trig_ips);

    // get the ip for this sensor and add it to the list
    IPAddress sensor_ip;
    if(get_ip_from_mac(sensor_macs[i], sensor_ip))
    {
      unit_ips[num_units++] = sensor_ip;
    }

    // add the sensor's trigger ips to the list
    for(int j=0; j<num_trigs; ++j)
    {
      unit_ips[num_units++] = trig_ips[j];
    }
  }

  return num_units>0;
}


///////////////////////////////////////////
// ShootNetRegister::get_mapped_triggers //
///////////////////////////////////////////
bool ShootNetRegister::get_mapped_triggers(const String& sensor_mac, unsigned& num_triggers, IPAddress* trigger_ips)
{
  bool found = false;

  for(int i=0; i<_num_sensors; ++i)
  {
    if(sensor_mac.equals(_sensor_map[i].sensor_mac))
    {
      found = true;
      num_triggers = _sensor_map[i].num_triggers;
      for(int j=0; j<num_triggers; ++j)
      {
        IPAddress trig_ip;
        if(get_ip_from_mac(_sensor_map[i].trigger_macs[j], trig_ip))
        {
          trigger_ips[j] = trig_ip;
        }
      }
      break;
    }
  }

  return found;
}


///////////////////////////////////////
// ShootNetRegister::get_stage_units //
///////////////////////////////////////
void ShootNetRegister::get_stage_units(unsigned stage_idx, String* units, unsigned& num_units)
{
  num_units = 0;

  for(int i=0; i<_stage_map[stage_idx].num_sensors; ++i)
  {
    Serial.printf( "S:%s\n", _stage_map[stage_idx].sensor_macs[i].c_str());
    //add_item("S:"+_stage_map[stage_idx].sensor_macs[i], MENU_NONE);
    units[num_units++] = _stage_map[stage_idx].sensor_macs[i];

    for(int j=0; j<_num_sensors; ++j)
    {
      if(_sensor_map[j].sensor_mac.equals(_stage_map[stage_idx].sensor_macs[i]))
      {
        for(int k=0; k<_sensor_map[j].num_triggers; ++k)
        {
          Serial.printf( "T:%s\n", _sensor_map[j].trigger_macs[k].c_str());
          units[num_units++] = _sensor_map[j].trigger_macs[k];
        }
        break;
      }
    }
  }
}


///////////////////////////////
// ShootNetRegister::trigger //
///////////////////////////////
void ShootNetRegister::trigger(const String& mac)
{
  for(int i=0; i<_num_sensors; ++i)
  {
    if(mac.equals(_sensor_map[i].sensor_mac))
    {
      _sensor_map[i].trigger_time = millis();
      break;
    }
  }
}


///////////////////////////////////////
// ShootNetRegister::get_ip_from_mac //
///////////////////////////////////////
bool ShootNetRegister::get_ip_from_mac(const String& mac, IPAddress& ip)
{
  bool found = false;
  for(int i=0; i<_num_units; ++i)
  {
    if(mac.equals(_register_map[i].mac))
    {
      found = true;
      ip = _register_map[i].ip;
      break;
    }
  }
  return found;
}


/////////////////////////////
// ShootNetRegister::print //
/////////////////////////////
void ShootNetRegister::print() const
{
  if(_num_units > 0)
  {
    Serial.println("register map:");
    for(int i=0; i<_num_units; ++i)
    {
      Serial.printf("  %s  %s  ",
        _register_map[i].mac.c_str(),
        unit_type_string(_register_map[i].unit_type).c_str());
      Serial.println(_register_map[i].ip);
    }

    Serial.println("stage->sensor map:");
    for(int i=0; i<MAX_STAGES; ++i)
    {
      if(_stage_map[i].num_sensors > 0)
      {
        Serial.printf(" Stage %d:\n", (i+1));
        for(int j=0; j<_stage_map[i].num_sensors; ++j)
        {
          Serial.printf( "   %s\n", _stage_map[i].sensor_macs[j].c_str());
        }
      }
    }

    Serial.println("sensor->trigger map:");
    for(int i=0; i<_num_sensors; ++i)
    {
      if(_sensor_map[i].num_triggers > 0)
      {
        Serial.printf(" %s:\n", _sensor_map[i].sensor_mac.c_str());
        for(int j=0; j<_sensor_map[i].num_triggers; ++j)
        {
          Serial.printf( "   %s\n", _sensor_map[i].trigger_macs[j].c_str());
        }
      }
    }
  }
  else
  {
    Serial.println(" EMPTY");
  }
  Serial.println("--------------------------");
}


