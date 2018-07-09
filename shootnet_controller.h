#ifndef SHOOTNET_CONTROLLER_h
#define SHOOTNET_CONTROLLER_h


#include <shootnet_unit.h>
#include <shootnet_menu.h>
#include <shootnet_button.h>
#include <shootnetregister.h>


class ShootNet_Controller: public ShootNet_Unit
{
  // make it easier to manage the controller responses from the menu
  friend class ShootNet_Menu;
  friend class ShootNet_MenuScreen;
  friend class Screen_Setup_Stg_Sns;
  friend class Screen_Setup_Unt_Delay;
  friend class Screen_Setup_Unt_Delay_Set;
  friend class Screen_Setup_Unt_Asgn;
  friend class Screen_Setup_Unt_Asgn_Trg;
  friend class Screen_Setup_Unt_Locate;
  friend class Screen_Stages_Action_Stat;
  friend class Screen_Status;

public:
  ShootNet_Controller();
  virtual void begin();
  virtual void iterate();
  virtual void configure_send_msg();
  virtual void handle_rcv_messsage(IPAddress remote_ip);
  void run_stage(unsigned idx);
  void reset_stage(unsigned idx);
  void locate(const String& mac);
  void check_triggers();

protected:
  ShootNetRegister _client_units; // contains various maps used to manage sensor/triggers/stages
  ShootNet_Menu    _menu;
  ShootNet_Button  _button_up;    // up
  ShootNet_Button  _button_dn;    // down
  ShootNet_Button  _button_sel;   // select
  IPAddress        _locating_ip;  // IP address of unit currently being "located" from the menu
};
#endif