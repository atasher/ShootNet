#ifndef SHOOTNET_TRIGGER_h
#define SHOOTNET_TRIGGER_h

#include <shootnet_client.h>



class ShootNet_Trigger: public ShootNet_Client
{
public:
  ShootNet_Trigger();

  virtual void begin();

  virtual void iterate();

  virtual void configure_send_msg();

  virtual void handle_rcv_messsage(IPAddress remote_ip);

  void check_trigger();

protected:
  bool     _triggered;    // true when this unit is triggered
  unsigned _trigger_time; // system time (ms) at time of trigger

};
#endif