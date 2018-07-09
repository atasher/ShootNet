#ifndef SHOOTNET_UNIT_h
#define SHOOTNET_UNIT_h

#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <shootnetmsg.h>
#include <shootnet_config.h>

#define LED_BLUE_PIN  14  // D5 on wemos
#define LED_GREEN_PIN 12  // D6 on wemos
#define LED_RED_PIN   13  // D7 on wemos





const unsigned RESP_TOLERANCE = 5000; // ms before declaring no response from controller


class ShootNet_Unit
{
public:
  ShootNet_Unit();

  virtual void begin();
  virtual void iterate();
  virtual void configure_send_msg();
  void print_connection_msg();
  virtual void setup_wifi();
  bool validate_rcv_msg(IPAddress remote_ip);
  bool send_message(MessageId id, IPAddress target_ip=CONTROLLER_IP);
  void check_for_message();
  bool handle_ack_message();
  virtual void handle_rcv_messsage(IPAddress remote_ip) = 0;
  virtual void heartbeat();
  void check_connection();

protected:
  unsigned _t_now; // current time in ms
  unsigned _last_heartbeat = 0; // time in ms

  WiFiServer  _server;
  ShootNetMsg _send_msg;
  ShootNetMsg _rcv_msg;
};
#endif