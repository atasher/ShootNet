#include <shootnet_menu.h>
#include <shootnet_controller.h>
#include <Arduino.h>


// selection and cursor characters are handled separately from each menu row content
//  so when the content is retrieved, it only contains relevant characters
const String MENU_PREFIX_UNSEL =  "  ";
const String MENU_PREFIX_SEL   =  " *";
const char   MENU_CURSOR       = '>';
const int    MENU_FIRST_ROW    = 7; // pixel row (from top) of the start of the first text row
const int    MENU_ROW_HEIGHT   = 9; // height (in pixels) of each text row


///////////////////
// ShootNet_Menu //
///////////////////


//////////////////////////////////
// ShootNet_Menu::ShootNet_Menu //
//////////////////////////////////
ShootNet_Menu::ShootNet_Menu(ShootNet_Controller& ctrl)
: _lcd(U8G2_R0, /* reset=*/ U8X8_PIN_NONE),
  _controller(ctrl),
  _current_row_index(0),
  _last_row_index(0),
  _last_row_content(""),
  _stage_selection(0),
  _sensor_selection(""),
  _current_screen(NULL)
{
  for(int i=0; i<MENU_MAX_LINES; ++i)
  {
    _items_selected[i] = false;
  }

  _screens[MENU_HOME] = new Screen_Home(ctrl);
  _screens[MENU_SETUP] = new Screen_Setup(ctrl);
  _screens[MENU_SETUP_STAGES] = new Screen_Setup_Stg(ctrl);
  _screens[MENU_SETUP_STAGES_SENS] = new Screen_Setup_Stg_Sns(ctrl);
  _screens[MENU_SETUP_UNITS] = new Screen_Setup_Unt(ctrl);
  _screens[MENU_SETUP_UNITS_DELAY] = new Screen_Setup_Unt_Delay(ctrl);
  _screens[MENU_SETUP_UNITS_DELAY_SET] = new Screen_Setup_Unt_Delay_Set(ctrl, _sensor_selection);
  _screens[MENU_SETUP_UNITS_ASSIGN] = new Screen_Setup_Unt_Asgn(ctrl);
  _screens[MENU_SETUP_UNITS_ASSIGN_TRIGS] = new Screen_Setup_Unt_Asgn_Trg(ctrl);
  _screens[MENU_SETUP_UNITS_LOCATE] = new Screen_Setup_Unt_Locate(ctrl);
  _screens[MENU_STAGES] = new Screen_Stages(ctrl);
  _screens[MENU_STAGES_ACTION] = new Screen_Stages_Action(ctrl);
  _screens[MENU_STAGES_ACTION_STATUS] = new Screen_Stages_Action_Stat(ctrl, _stage_selection);
  _screens[MENU_STATUS] = new Screen_Status(ctrl);
}


//////////////////////////
// ShootNet_Menu::setup //
//////////////////////////
void ShootNet_Menu::setup()
{
  _lcd.begin();
  _lcd.setFont(u8g2_font_6x12_tr);

  // start menu on home screen
  draw(*_screens[MENU_HOME]);
}


/////////////////////////
// ShootNet_Menu::draw //
/////////////////////////
void ShootNet_Menu::draw(ShootNet_MenuScreen& screen)
{
  // save last index and row content in case either is needed for the next screen
  _last_row_index = _current_row_index;
  if(_current_screen != NULL) // will only be null on first draw
  {
    _last_row_content = _current_screen->get_content()[_last_row_index];
    Serial.printf("last row content: %s\n", _last_row_content.c_str());
  }

  _current_screen = &screen;
  _current_row_index = 0;

  redraw();
}


///////////////////////////
// ShootNet_Menu::redraw //
///////////////////////////
void ShootNet_Menu::redraw()
{
  _lcd.clearBuffer();

  const String* content = _current_screen->get_content();

  // this handles scrolling if more than max number of rows of content exist
  unsigned istart = 0;
  if(_current_row_index >= MENU_MAX_DISPLAY)
  {
    istart = _current_row_index - MENU_MAX_DISPLAY + 1;
  }

  unsigned line = 0;
  for(unsigned i=istart; i<_current_screen->get_lines(); ++i,++line)
  {
    // add selection and cursor characters where applicable
    String curr_line;
    if(is_selected(i))
    {
      curr_line = MENU_PREFIX_SEL+content[i];
    }
    else
    {
      curr_line = MENU_PREFIX_UNSEL+content[i];
    }
    if(i == _current_row_index)
    {
      curr_line.setCharAt(0, MENU_CURSOR);
    }
    _lcd.drawStr(0,MENU_FIRST_ROW+line*MENU_ROW_HEIGHT, curr_line.c_str());
  }

  _lcd.sendBuffer();

  // if the current line contains a mac address, the corresponding unit will start blinking
  _controller.locate(get_current_line());
}


///////////////////////
// ShootNet_Menu::up //
///////////////////////
void ShootNet_Menu::up()
{
  // the delay screen is unique in that it scrolls the delay time rather than
  //  moving the cursor up or down
  if(MENU_SETUP_UNITS_DELAY_SET == _current_screen->get_id())
  {
    for(int i=0; i<_controller._client_units._num_sensors; ++i)
    {
      if(_sensor_selection.equals(_controller._client_units._sensor_map[i].sensor_mac))
      {
        if(_controller._client_units._sensor_map[i].trigger_delay < 10000)
        {
          _controller._client_units._sensor_map[i].trigger_delay += 500;
        }
        break;
      }
    }
  }
  // normal case; move cursor up
  else
  {
    if(_current_row_index > 0)
    {
      _current_row_index -= 1;
    }
    else
    {
      _current_row_index = _current_screen->get_lines()-1;
    }
  }

  redraw();
}


/////////////////////////
// ShootNet_Menu::down //
/////////////////////////
void ShootNet_Menu::down()
{
  // the delay screen is unique in that it scrolls the delay time rather than
  //  moving the cursor up or down
  if(MENU_SETUP_UNITS_DELAY_SET == _current_screen->get_id())
  {
    for(int i=0; i<_controller._client_units._num_sensors; ++i)
    {
      if(_sensor_selection.equals(_controller._client_units._sensor_map[i].sensor_mac))
      {
        if(_controller._client_units._sensor_map[i].trigger_delay >= 500)
        {
          _controller._client_units._sensor_map[i].trigger_delay -= 500;
        }
        break;
      }
    }
  }
  // normal case; move cursor down
  else
  {
    if(_current_row_index < _current_screen->get_lines()-1)
    {
      _current_row_index += 1;
    }
    else
    {
      _current_row_index = 0;
    }
  }

  redraw();
}


///////////////////////////
// ShootNet_Menu::select //
///////////////////////////
void ShootNet_Menu::select()
{
  // id of the next screen, which is based on the currently selected item
  MenuId next_id = _current_screen->get_next_id(_current_row_index);

  // If no next screen; this is (usually) a selection screen
  if(MENU_NONE == next_id)
  {
    //toggle menu item selection
    if(MENU_SETUP_STAGES_SENS == _current_screen->get_id())
    {
      if(_controller._client_units.map_stage_sensor(_last_row_index, _current_screen->_items[_current_row_index]))
      {
        Serial.printf("SELECT %s FOR STAGE %d\n", _current_screen->_items[_current_row_index].c_str(), (_last_row_index+1));
        _items_selected[_current_row_index] = true;
      }
      else
      {
        Serial.printf("UNSELECT %s FROM STAGE %d\n", _current_screen->_items[_current_row_index].c_str(), (_last_row_index+1));
        _items_selected[_current_row_index] = false;
      }
      _controller._client_units.print();
      redraw();
    }
    else if(MENU_SETUP_UNITS_ASSIGN_TRIGS == _current_screen->get_id())
    {
      if(_controller._client_units.map_sensor_trigger(_last_row_content, _current_screen->_items[_current_row_index]))
      {
        Serial.printf("SELECT %s FOR SENSOR %s\n", _current_screen->_items[_current_row_index].c_str(), _last_row_content.c_str());
        _items_selected[_current_row_index] = true;
      }
      else
      {
        Serial.printf("UNSELECT %s FROM SENSOR %s\n", _current_screen->_items[_current_row_index].c_str(), _last_row_content.c_str());
        _items_selected[_current_row_index] = false;
      }
      _controller._client_units.print();
      redraw();
    }
  }
  // current screen has a next screen
  else
  {
    for(int i=0; i<NUM_SCREENS; ++i)
    {
      MenuId iter_id = _screens[i]->get_id();
      if(iter_id == next_id)
      {
        // get selections for this screen, if applicable to this id
        if(MENU_SETUP_STAGES_SENS == next_id)
        {
          Serial.printf("setting stage map selections for STAGE %d\n", (_current_row_index+1));
          _controller._client_units.set_stage_map_selections(_current_row_index, _screens[i]->_items, _screens[i]->get_lines(), _items_selected);
        }
        else if(MENU_SETUP_UNITS_DELAY_SET == next_id)
        {
          _sensor_selection = get_current_line();
          Serial.printf("Sensor %s selected (last=%s)\n", _sensor_selection.c_str(), _last_row_content.c_str());
        }
        else if(MENU_SETUP_UNITS_ASSIGN_TRIGS == next_id)
        {
          Serial.printf("setting sensor map selections for SENSOR %s\n", _current_screen->get_content()[_current_row_index].c_str());
          _controller._client_units.set_sensor_map_selections(_current_screen->get_content()[_current_row_index], _screens[i]->_items, _screens[i]->get_lines(), _items_selected);
        }
        // only set _stage_selection when going from MENU_STAGES (as opposed to going back from MENU_STAGES_ACTION_STATUS
        else if(MENU_STAGES_ACTION == next_id && MENU_STAGES == _current_screen->get_id())
        {
          _stage_selection = _current_row_index;
          Serial.printf("Stage %d selected (last=%s)\n", (_stage_selection+1), _last_row_content.c_str());
        }
        else if(MENU_STAGES_ACTION_STATUS == next_id)
        {
          Serial.printf("%s Stage %d\n", get_current_line().c_str(), (_stage_selection+1));
          if(get_current_line().equals("Run"))
          {
            // get ip of sensors mapped to _stage_selection
            _controller.run_stage(_stage_selection);
          }
          else if(get_current_line().equals("Reset"))
          {
            _controller.reset_stage(_stage_selection);
          }
          // do nothing for "Status" (status screen will be shown anyway)
        }
        // for all other screens, clear all row selections
        else
        {
          for(int i=0; i<MENU_MAX_LINES; ++i)
          {
            _items_selected[i] = false;
          }
        }

        draw(*_screens[i]);
        break; // out of for loop
      }
    }
  }
}



/////////////////////////
// ShootNet_MenuScreen //
/////////////////////////



//////////////////////////////////////////////
// ShootNet_MenuScreen::ShootNet_MenuScreen //
//////////////////////////////////////////////
ShootNet_MenuScreen::ShootNet_MenuScreen(MenuId id, ShootNet_Controller& ctrl)
: _id(id),
  _controller(ctrl),
  _lines(0)
{
  for(int i=0; i<MENU_MAX_LINES; ++i)
  {
    _items[i] = "";
    _next_screen[i] = MENU_NONE;
  }
}


///////////////////////////////////
// ShootNet_MenuScreen::add_item //
///////////////////////////////////
void ShootNet_MenuScreen::add_item(String line, MenuId next_screen)
{
  _items[_lines] = line;
  _next_screen[_lines] = next_screen;
  _lines++;
}


//////////////////////////////
// Screen_Home::Screen_Home //
//////////////////////////////
Screen_Home::Screen_Home(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_HOME, ctrl)
{
  add_item("Setup",  MENU_SETUP);
  add_item("Stages", MENU_STAGES);
  add_item("Status", MENU_STATUS);
}


////////////////////////////////
// Screen_Setup::Screen_Setup //
////////////////////////////////
Screen_Setup::Screen_Setup(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP, ctrl)
{
  add_item("Stage Setup", MENU_SETUP_STAGES);
  add_item("Unit Setup",  MENU_SETUP_UNITS);
  add_item("<-BACK",      MENU_HOME);
}


////////////////////////////////////////
// Screen_Setup_Stg::Screen_Setup_Stg //
////////////////////////////////////////
Screen_Setup_Stg::Screen_Setup_Stg(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP_STAGES, ctrl)
{
  add_item("Stage 1", MENU_SETUP_STAGES_SENS);
  add_item("Stage 2", MENU_SETUP_STAGES_SENS);
  add_item("Stage 3", MENU_SETUP_STAGES_SENS);
  add_item("Stage 4", MENU_SETUP_STAGES_SENS);
  add_item("<-BACK",  MENU_SETUP);
}


////////////////////////////////////////
// Screen_Setup_Unt::Screen_Setup_Unt //
////////////////////////////////////////
Screen_Setup_Unt::Screen_Setup_Unt(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP_UNITS, ctrl)
{
  add_item("Set Sensor Delays", MENU_SETUP_UNITS_DELAY);
  add_item("Assign Triggers",   MENU_SETUP_UNITS_ASSIGN);
  add_item("Locate Units",      MENU_SETUP_UNITS_LOCATE);
  add_item("<-BACK",            MENU_SETUP);
}


////////////////////////////////////////////////
// Screen_Setup_Stg_Sns::Screen_Setup_Stg_Sns //
////////////////////////////////////////////////
Screen_Setup_Stg_Sns::Screen_Setup_Stg_Sns(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP_STAGES_SENS, ctrl)
{}


////////////////////////////////////////////////////
// Screen_Setup_Unt_Delay::Screen_Setup_Unt_Delay //
////////////////////////////////////////////////////
Screen_Setup_Unt_Delay::Screen_Setup_Unt_Delay(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP_UNITS_DELAY, ctrl)
{}


////////////////////////////////////////////////////////////
// Screen_Setup_Unt_Delay_Set::Screen_Setup_Unt_Delay_Set //
////////////////////////////////////////////////////////////
Screen_Setup_Unt_Delay_Set::Screen_Setup_Unt_Delay_Set(ShootNet_Controller& ctrl, String& sensor)
: ShootNet_MenuScreen(MENU_SETUP_UNITS_DELAY_SET, ctrl),
  _sensor_mac(sensor)
{}


//////////////////////////////////////////////////
// Screen_Setup_Unt_Asgn::Screen_Setup_Unt_Asgn //
//////////////////////////////////////////////////
Screen_Setup_Unt_Asgn::Screen_Setup_Unt_Asgn(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP_UNITS_ASSIGN, ctrl)
{}


//////////////////////////////////////////////////////////
// Screen_Setup_Unt_Asgn_Trg::Screen_Setup_Unt_Asgn_Trg //
//////////////////////////////////////////////////////////
Screen_Setup_Unt_Asgn_Trg::Screen_Setup_Unt_Asgn_Trg(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP_UNITS_ASSIGN_TRIGS, ctrl)
{}


//////////////////////////////////////////////////////
// Screen_Setup_Unt_Locate::Screen_Setup_Unt_Locate //
//////////////////////////////////////////////////////
Screen_Setup_Unt_Locate::Screen_Setup_Unt_Locate(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_SETUP_UNITS_LOCATE, ctrl)
{}


//////////////////////////////////////////////////////////
// Screen_Stages_Action_Stat::Screen_Stages_Action_Stat //
//////////////////////////////////////////////////////////
Screen_Stages_Action_Stat::Screen_Stages_Action_Stat(ShootNet_Controller& ctrl, unsigned& stage)
: ShootNet_MenuScreen(MENU_STAGES_ACTION_STATUS, ctrl),
  _stage(stage)
{}

//////////////////////////////////
// Screen_Status::Screen_Status //
//////////////////////////////////
Screen_Status::Screen_Status(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_STATUS, ctrl)
{}


//////////////////////////////////
// Screen_Stages::Screen_Stages //
//////////////////////////////////
Screen_Stages::Screen_Stages(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_STAGES, ctrl)
{
  add_item("Stage 1", MENU_STAGES_ACTION);
  add_item("Stage 2", MENU_STAGES_ACTION);
  add_item("Stage 3", MENU_STAGES_ACTION);
  add_item("Stage 4", MENU_STAGES_ACTION);
  add_item("<-BACK",  MENU_HOME);
}


////////////////////////////////////////////////
// Screen_Stages_Action::Screen_Stages_Action //
////////////////////////////////////////////////
Screen_Stages_Action::Screen_Stages_Action(ShootNet_Controller& ctrl)
: ShootNet_MenuScreen(MENU_STAGES_ACTION, ctrl)
{
  add_item("Run",    MENU_STAGES_ACTION_STATUS);
  add_item("Reset",  MENU_STAGES_ACTION_STATUS);
  add_item("Status", MENU_STAGES_ACTION_STATUS);
  add_item("<-BACK", MENU_STAGES);
}


///////////////////////////////////////
// Screen_Setup_Stg_Sns::get_content //
///////////////////////////////////////
const String* Screen_Setup_Stg_Sns::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  for(int i=0; i<_controller._client_units._num_units; ++i)
  {
    if(UNIT_TYPE_SENSOR == _controller._client_units._register_map[i].unit_type)
    {
      add_item(_controller._client_units._register_map[i].mac, MENU_NONE);
    }
  }

  add_item("<-BACK", MENU_SETUP_STAGES);

  return _items;
}


/////////////////////////////////////////
// Screen_Setup_Unt_Delay::get_content //
/////////////////////////////////////////
const String* Screen_Setup_Unt_Delay::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  for(int i=0; i<_controller._client_units._num_sensors; ++i)
  {
    add_item(_controller._client_units._sensor_map[i].sensor_mac, MENU_SETUP_UNITS_DELAY_SET);
  }

  add_item("<-BACK", MENU_SETUP_UNITS);

  return _items;
}


/////////////////////////////////////////////
// Screen_Setup_Unt_Delay_Set::get_content //
/////////////////////////////////////////////
const String* Screen_Setup_Unt_Delay_Set::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  for(int i=0; i<_controller._client_units._num_sensors; ++i)
  {
    if(_sensor_mac.equals(_controller._client_units._sensor_map[i].sensor_mac))
    {
      add_item(String(_controller._client_units._sensor_map[i].trigger_delay), MENU_SETUP_UNITS_DELAY);
      break;
    }
  }

  return _items;
}


////////////////////////////////////////
// Screen_Setup_Unt_Asgn::get_content //
////////////////////////////////////////
const String* Screen_Setup_Unt_Asgn::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  for(int i=0; i<_controller._client_units._num_sensors; ++i)
  {
    add_item(_controller._client_units._sensor_map[i].sensor_mac, MENU_SETUP_UNITS_ASSIGN_TRIGS);
  }

  add_item("<-BACK", MENU_SETUP_UNITS);

  return _items;
}


////////////////////////////////////////////
// Screen_Setup_Unt_Asgn_Trg::get_content //
////////////////////////////////////////////
const String* Screen_Setup_Unt_Asgn_Trg::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  for(int i=0; i<_controller._client_units._num_units; ++i)
  {
    if(UNIT_TYPE_TRIGGER == _controller._client_units._register_map[i].unit_type)
    {
      add_item(_controller._client_units._register_map[i].mac, MENU_NONE);
    }
  }

  add_item("<-BACK", MENU_SETUP_UNITS_ASSIGN);

  return _items;
}


//////////////////////////////////////////
// Screen_Setup_Unt_Locate::get_content //
//////////////////////////////////////////
const String* Screen_Setup_Unt_Locate::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  for(int i=0; i<_controller._client_units._num_units; ++i)
  {
    add_item(_controller._client_units._register_map[i].mac, MENU_NONE);
  }

  add_item("<-BACK", MENU_SETUP_UNITS);

  return _items;
}


////////////////////////////////////////////
// Screen_Stages_Action_Stat::get_content //
////////////////////////////////////////////
const String* Screen_Stages_Action_Stat::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  String stage_macs[MAX_UNITS];
  unsigned num_macs;

  _controller._client_units.get_stage_units(_stage, stage_macs, num_macs);

  add_item(String("Stage ")+(_stage+1), MENU_NONE);

  for(int i=0; i<num_macs; ++i)
  {
    add_item(stage_macs[i], MENU_NONE);
  }

  add_item("<-BACK", MENU_STAGES_ACTION);

  return _items;
}


////////////////////////////////
// Screen_Status::get_content //
////////////////////////////////
const String* Screen_Status::get_content()
{
  // reset the content since it is generated dynamically
  _lines = 0;

  add_item("Ctrl:"+CONTROLLER_IP.toString(), MENU_NONE);
  add_item(String("Sensors:")+_controller._client_units.num_sensors(), MENU_NONE);
  add_item(String("Triggers:")+_controller._client_units.num_triggers(), MENU_NONE);

  add_item("<-BACK", MENU_HOME);

  return _items;
}
