#ifndef SHOOTNET_BUTTON_h
#define SHOOTNET_BUTTON_h

class ShootNet_Button
{
public:
  ShootNet_Button(int pin);

  bool pressed();

private:
  unsigned _pin;
  unsigned _t_bpress;
  int      _b_state_prev;
  int      _b_state;
};
#endif