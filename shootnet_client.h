#ifndef SHOOTNET_CLIENT_h
#define SHOOTNET_CLIENT_h

#include <shootnet_unit.h>


// Number of times a heartbeat must fail before client resets itself
extern const unsigned HB_FAULT_LIMIT;



class ShootNet_Client : public ShootNet_Unit
{
public:
  ShootNet_Client();
  virtual void begin();
  virtual void heartbeat();
  void check_blink();

protected:
  unsigned _hb_fault_count; // number of consecutive heartbeat faults
  unsigned _last_blink;     // last blink time (system time, ms)
  bool     _armed;          // true when this unit is armed
  bool     _locating;       // true when this unit has the menu cursor on it
  bool     _blink_on;       // flag used to blink the locate or armed LEDs
};

#endif