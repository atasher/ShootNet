#ifndef SHOOTNETREGISTER_h
#define SHOOTNETREGISTER_h


#include <IPAddress.h>
#include <shootnet_types.h>

class ShootNetMsg;


const unsigned MAX_UNITS  = 16;
const unsigned MAX_STAGES = 4;


class ShootNetRegister
{
public: // public methods
  ShootNetRegister();

  void register_unit(ShootNetMsg& msg, IPAddress ip);

  bool map_stage_sensor(unsigned stage_idx, const String& sensor_mac);

  bool map_sensor_trigger(const String& sensor_mac, const String& trigger_mac);

  void set_stage_map_selections(unsigned stage_idx, String* macs, unsigned num_lines, bool* selections);

  void set_sensor_map_selections(const String& sensor_mac, String* trigger_macs, unsigned num_lines, bool* selections);

  bool get_stage_ips(unsigned stage_idx, unsigned& num_units, IPAddress* unit_ips);

  bool get_mapped_triggers(const String& sensor_mac, unsigned& num_triggers, IPAddress* trigger_ips);

  void get_stage_units(unsigned stage_idx, String* units, unsigned& num_units);

  void trigger(const String& mac);

  bool get_ip_from_mac(const String& mac, IPAddress& ip);

  void print() const;

  unsigned num_sensors() const{return _num_sensors;}
  unsigned num_triggers() const{return _num_units-_num_sensors;}


private: // private methods


public: // private members

  unsigned _num_units;   // number of total sensors + triggers
  unsigned _num_sensors; // number of sensors only

  struct
  {
    String    mac;
    IPAddress ip;
    UnitType  unit_type;
  } _register_map[MAX_UNITS];

  // Maps stages with their assigned sensors
  struct // stage number is the index+1
  {
    unsigned num_sensors;
    String   sensor_macs[MAX_UNITS];
  } _stage_map[MAX_STAGES];

  // Maps sensors with their assigned triggers
  struct
  {
    unsigned trigger_time;
    unsigned trigger_delay;
    String   sensor_mac;
    unsigned num_triggers;
    String   trigger_macs[MAX_UNITS];
  } _sensor_map[MAX_UNITS];



};
#endif