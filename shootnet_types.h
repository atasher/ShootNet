#ifndef SHOOTNET_TYPES_h
#define SHOOTNET_TYPES_h

// note: REGISTER is no longer used; HEARTBEAT is used to register a unit if it
//        is found to be new/changed
enum MessageId
{
  MSG_ID_HEARTBEAT,     // sensor/trigger -> ctrl
  MSG_ID_ACK,           // ctrl <-> sensor/trigger
  MSG_ID_ARM,           // ctrl -> sensor/trigger
  MSG_ID_RESET,         // ctrl -> sensor/trigger
  MSG_ID_LOCATE_ON,     // ctrl -> sensor/trigger
  MSG_ID_LOCATE_OFF,    // ctrl -> sensor/trigger
  MSG_ID_SENSOR_DETECT, // sensor -> ctrl
  MSG_ID_TRIGGER_FIRE,  // ctrl - >trigger

  // ***Do not add below this entry***
  MSG_ID_COUNT
};

enum UnitType
{
  UNIT_TYPE_CONTROLLER,
  UNIT_TYPE_TRIGGER,
  UNIT_TYPE_SENSOR
};

static String unit_type_string(UnitType type)
{
  if(UNIT_TYPE_TRIGGER == type)
  {
    return "TRIGGER";
  }
  else if(UNIT_TYPE_SENSOR == type)
  {
    return "SENSOR";
  }
  else
  {
    return "CONTROLLER";
  }
}

///////////////////
// mac_to_string //
///////////////////
static String mac_to_string(const unsigned char* mac)
{
  char buf[20];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

#endif