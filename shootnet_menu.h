#ifndef SHOOTNET_MENU_h
#define SHOOTNET_MENU_h

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


class ShootNet_Controller;


const int MENU_MAX_LINES = 20;
const int MENU_MAX_DISPLAY = 7; // max lines that can be displayed with current font; use for scrolling


enum MenuId
{
  MENU_HOME,                     // Setup, Stages, Status
  MENU_SETUP,                    // Stage Setup, Unit Setup
  MENU_SETUP_STAGES,             // Stage 1, ... , Stage 4
  MENU_SETUP_STAGES_SENS,        // [list all sensor macs]
  MENU_SETUP_UNITS,              // Assign Triggers, Locate Units
  MENU_SETUP_UNITS_DELAY,        // [list all sensor macs]
  MENU_SETUP_UNITS_DELAY_SET,    // set delay in 0.5s increments
  MENU_SETUP_UNITS_ASSIGN,       // [list all sensor macs]
  MENU_SETUP_UNITS_ASSIGN_TRIGS, // [list all trigger macs]
  MENU_SETUP_UNITS_LOCATE,       // [list all macs]
  MENU_STAGES,                   // Stage 1, ... , Stage 4
  MENU_STAGES_ACTION,            // Run, Reset, Status
  MENU_STAGES_ACTION_STATUS,     // [show stage number and assigned sensor/trigger macs]
  MENU_STATUS,                   // [show ctrl IP and number of connected units]

  MENU_NONE, // used for screens that don't have a next screen
};


class ShootNet_MenuScreen
{
  friend class ShootNet_Menu;

public:
  ShootNet_MenuScreen(MenuId id, ShootNet_Controller& ctrl);
  void add_item(String line, MenuId next_screen);
  virtual const String* get_content(){return _items;}
  unsigned get_lines() const{return _lines;}
  MenuId get_id() const{return _id;}
  MenuId get_next_id(unsigned idx) const{return _next_screen[idx];}


protected:
  MenuId               _id;
  ShootNet_Controller& _controller;
  String               _items[MENU_MAX_LINES];
  MenuId               _next_screen[MENU_MAX_LINES];
  unsigned             _lines;
};


class Screen_Home :                public ShootNet_MenuScreen{public:                Screen_Home(ShootNet_Controller& ctrl);};
class Screen_Setup :               public ShootNet_MenuScreen{public:               Screen_Setup(ShootNet_Controller& ctrl);};
class Screen_Setup_Stg :           public ShootNet_MenuScreen{public:           Screen_Setup_Stg(ShootNet_Controller& ctrl);};
class Screen_Setup_Stg_Sns :       public ShootNet_MenuScreen{public:       Screen_Setup_Stg_Sns(ShootNet_Controller& ctrl);  virtual const String* get_content();};
class Screen_Setup_Unt :           public ShootNet_MenuScreen{public:           Screen_Setup_Unt(ShootNet_Controller& ctrl);};
class Screen_Setup_Unt_Delay :     public ShootNet_MenuScreen{public:     Screen_Setup_Unt_Delay(ShootNet_Controller& ctrl);  virtual const String* get_content();};
class Screen_Setup_Unt_Delay_Set : public ShootNet_MenuScreen{public: Screen_Setup_Unt_Delay_Set(ShootNet_Controller& ctrl,String& sensor);  virtual const String* get_content(); String& _sensor_mac;};
class Screen_Setup_Unt_Asgn :      public ShootNet_MenuScreen{public:      Screen_Setup_Unt_Asgn(ShootNet_Controller& ctrl);  virtual const String* get_content();};
class Screen_Setup_Unt_Asgn_Trg :  public ShootNet_MenuScreen{public:  Screen_Setup_Unt_Asgn_Trg(ShootNet_Controller& ctrl);  virtual const String* get_content();};
class Screen_Setup_Unt_Locate :    public ShootNet_MenuScreen{public:    Screen_Setup_Unt_Locate(ShootNet_Controller& ctrl);  virtual const String* get_content();};
class Screen_Stages :              public ShootNet_MenuScreen{public:              Screen_Stages(ShootNet_Controller& ctrl);};
class Screen_Stages_Action :       public ShootNet_MenuScreen{public:       Screen_Stages_Action(ShootNet_Controller& ctrl);};
class Screen_Stages_Action_Stat :  public ShootNet_MenuScreen{public:  Screen_Stages_Action_Stat(ShootNet_Controller& ctrl,unsigned& stage);  virtual const String* get_content(); unsigned& _stage;};
class Screen_Status :              public ShootNet_MenuScreen{public:              Screen_Status(ShootNet_Controller& ctrl);  virtual const String* get_content();};

// This must be the same number of screen subclasses
const unsigned NUM_SCREENS = 14;


class ShootNet_Menu
{
public:
  ShootNet_Menu(ShootNet_Controller& ctrl);

  void setup();
  void draw(ShootNet_MenuScreen& screen);
  void redraw();
  bool is_selected(unsigned idx) const{return _items_selected[idx];}
  const String& get_current_line() const{return _current_screen->get_content()[_current_row_index];}
  void up();
  void down();
  void select();

private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C _lcd;
  ShootNet_Controller&               _controller;
  unsigned                           _current_row_index; // currently highlighted row
  unsigned                           _last_row_index;    // highlighted row in previous menu screen
  String                             _last_row_content;  // highlighted content in previous menu screen
  unsigned                           _stage_selection;   // selected stage number
  String                             _sensor_selection;  // selected sensor mac
  ShootNet_MenuScreen*               _current_screen;    // pointer to current screen in _screens
  bool                               _items_selected[MENU_MAX_LINES]; // corresponds to selected rows in current screen

  ShootNet_MenuScreen* _screens[NUM_SCREENS]; // contains pointers to all used menu screen objects
};

#endif