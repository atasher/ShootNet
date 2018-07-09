#ifndef SHOOTNET_SENSOR_h
#define SHOOTNET_SENSOR_h

#include <shootnet_client.h>


class ShootNet_Sensor: public ShootNet_Client
{
public:
  ShootNet_Sensor();

  virtual void begin();
  virtual void iterate();
  virtual void configure_send_msg();
  virtual void handle_rcv_messsage(IPAddress remote_ip);
};
#endif