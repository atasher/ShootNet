#ifndef SHOOTNETMSG_h
#define SHOOTNETMSG_h

#include <shootnet_types.h>


#define MAC_SIZE 6  // size in bytes
#define IP_SIZE 4   // size in bytes
#define SHOOTNET_MSG_SIZE 2+MAC_SIZE+IP_SIZE // size in bytes


class IPAddress;


class ShootNetMsg
{
public:
  ShootNetMsg();

  void set_unit_type(UnitType type);
  void set_mac(uint8_t mac[6]);
  void set_ip(IPAddress ip);
  IPAddress get_ip() const;
  void set_msg(MessageId id);
  void print() const;

  inline MessageId get_id() const
  {
    return (MessageId)_message.fields.id;
  }

  inline const uint8_t* get_mac() const
  {
    return _message.fields.mac;
  }

  inline UnitType get_unit_type() const
  {
    return (UnitType)_message.fields.unit_type;
  }

  inline const uint8_t* raw_data() const
  {
    return _message.bytes;
  }

  inline uint8_t* rcv_data()
  {
    return _message.bytes;
  }


private:

  union
  {
    uint8_t bytes[SHOOTNET_MSG_SIZE];
    struct
    {
      uint8_t id;
      uint8_t unit_type;
      uint8_t mac[MAC_SIZE];
      uint8_t ip[IP_SIZE];
    } fields;
  } _message;
};
#endif
