#include <Arduino.h>
#include <IPAddress.h>
#include <shootnetmsg.h>


//////////////////////////////
// ShootNetMsg::ShootNetMsg //
//////////////////////////////
ShootNetMsg::ShootNetMsg()
{
  for(int i=0; i<SHOOTNET_MSG_SIZE; ++i)
  {
    _message.bytes[0] = 0;
  }
}


////////////////////////////////
// ShootNetMsg::set_unit_type //
////////////////////////////////
void ShootNetMsg::set_unit_type(UnitType type)
{
  _message.fields.unit_type = type;
}


//////////////////////////
// ShootNetMsg::set_mac //
//////////////////////////
void ShootNetMsg::set_mac(uint8_t mac[6])
{
  for(int i=0; i<MAC_SIZE; ++i)
  {
    _message.fields.mac[i] = mac[i];
  }
}


/////////////////////////
// ShootNetMsg::set_ip //
/////////////////////////
void ShootNetMsg::set_ip(IPAddress ip)
{
  for(int i=0; i<IP_SIZE; ++i)
  {
    _message.fields.ip[i] = ip[i];
  }
}


/////////////////////////
// ShootNetMsg::get_ip //
/////////////////////////
IPAddress ShootNetMsg::get_ip() const
{
  IPAddress ip(_message.fields.ip);
  return ip;
}


//////////////////////////
// ShootNetMsg::set_msg //
//////////////////////////
void ShootNetMsg::set_msg(MessageId id)
{
  _message.fields.id = id;
}


////////////////////////
// ShootNetMsg::print //
////////////////////////
void ShootNetMsg::print() const
{
  for(int i=0; i<SHOOTNET_MSG_SIZE; ++i)
  {
    Serial.printf("%02X", _message.bytes[i]);
  }
  Serial.println();


  switch(_message.fields.id)
  {
    case MSG_ID_ACK:
      Serial.println(" Msg: ACK"); break;
    case MSG_ID_HEARTBEAT:
      Serial.println(" Msg: HEARTBEAT"); break;
    case MSG_ID_ARM:
      Serial.println(" Msg: ARM"); break;
    case MSG_ID_RESET:
      Serial.println(" Msg: RESET"); break;
    case MSG_ID_LOCATE_ON:
      Serial.println(" Msg: MSG_ID_LOCATE_ON"); break;
    case MSG_ID_LOCATE_OFF:
      Serial.println(" Msg: MSG_ID_LOCATE_OFF"); break;
    case MSG_ID_SENSOR_DETECT:
      Serial.println(" Msg: SENSOR_DETECT"); break;
    case MSG_ID_TRIGGER_FIRE:
      Serial.println(" Msg: TRIGGER_FIRE"); break;
  }

  Serial.print(" ");
  Serial.println(unit_type_string((UnitType)_message.fields.unit_type));


  Serial.print(" MAC: ");
  for(int i=0; i<MAC_SIZE; ++i)
  {
    Serial.printf("%02X", _message.fields.mac[i]);
    if(i<MAC_SIZE-1) Serial.printf(":");
  }
  Serial.println();

  Serial.printf("  IP: ");
  for(int i=0; i<IP_SIZE; ++i)
  {
    Serial.printf("%d", _message.fields.ip[i]);
    if(i<IP_SIZE-1) Serial.printf(".");
  }
  Serial.println();
  Serial.println("--------------------------");
}
