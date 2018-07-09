#include <shootnet_unit.h>
#include <WiFiClient.h>

const unsigned SERVER_PORT = 80;
const unsigned HB_INTERVAL = 10000; // ms

//////////////////////////////////
// ShootNet_Unit::ShootNet_Unit //
//////////////////////////////////
ShootNet_Unit::ShootNet_Unit()
: _server(SERVER_PORT)
{}


//////////////////////////
// ShootNet_Unit::begin //
//////////////////////////
void ShootNet_Unit::begin()
{
  pinMode(LED_BLUE_PIN, OUTPUT);

  digitalWrite(LED_BLUE_PIN, LOW);

  delay(10);

  setup_wifi();

  print_connection_msg();

  configure_send_msg();

  _server.begin();
}


///////////////////////////////
// ShootNet_Unit::setup_wifi //
///////////////////////////////
void ShootNet_Unit::setup_wifi()
{
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  // Wait for the Wi-Fi to connect
  do
  {
    digitalWrite(LED_BLUE_PIN, LOW);
    delay(500);
    digitalWrite(LED_BLUE_PIN, HIGH);
    Serial.print('.');
    delay(500);
  }while (WiFi.status() != WL_CONNECTED);

  Serial.println();
  Serial.println("   unit connected");
}


//////////////////////////////
// ShootNet_Unit::iterate() //
//////////////////////////////
void ShootNet_Unit::iterate()
{
  _t_now = millis();

  if(_t_now >= (_last_heartbeat+HB_INTERVAL))
  {
    heartbeat();
  }
}


/////////////////////////////////////////
// ShootNet_Unit::print_connection_msg //
/////////////////////////////////////////
void ShootNet_Unit::print_connection_msg()
{
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print(" LocalIP:"); Serial.println(WiFi.localIP());
  Serial.print("  Subnet:"); Serial.println(WiFi.subnetMask());
  Serial.print("     DNS:"); Serial.println(WiFi.dnsIP());
  Serial.print("     MAC:"); Serial.println(WiFi.macAddress());
  Serial.print(" Gateway:"); Serial.println(WiFi.gatewayIP());
  Serial.print("  AP MAC:"); Serial.println(WiFi.BSSIDstr());
  Serial.print("hostname:"); Serial.println(WiFi.hostname());
  Serial.print("  status:"); Serial.println(WiFi.status());
  Serial.print("    RSSI:"); Serial.println(WiFi.RSSI());
}


/////////////////////////////////////////
// ShootNet_Unit::configure_send_msg() //
/////////////////////////////////////////
void ShootNet_Unit::configure_send_msg()
{
  // set the mac and ip address for this unit in the send message
  uint8_t mac[6];
  WiFi.macAddress(mac);
  _send_msg.set_mac(mac);

  _send_msg.set_ip(WiFi.localIP());
}

///////////////////////////////////////
// ShootNet_Unit::validate_rcv_msg() //
///////////////////////////////////////
bool ShootNet_Unit::validate_rcv_msg(IPAddress remote_ip)
{
  // make sure the IP set in the message by the sender is the same
  //  as the remote IP given by the wifi library. they might be different
  //  if the receiving unit inadvertently reads stale data from its receive
  //  buffer
  if(_rcv_msg.get_ip() != remote_ip)
  {
    Serial.print("  received IP doesn't match remoteIP: ");
    Serial.println(remote_ip);
  }
  if(_rcv_msg.get_id() >= MSG_ID_COUNT)
  {
    Serial.printf("  invalid message ID: %d > msg count %d\n", _rcv_msg.get_id(), MSG_ID_COUNT);
  }

  return (_rcv_msg.get_ip() == remote_ip  &&  _rcv_msg.get_id() < MSG_ID_COUNT);
}


///////////////////////////////////
// ShootNet_Unit::send_message() //
///////////////////////////////////
bool ShootNet_Unit::send_message(MessageId id, IPAddress target_ip)
{
  bool success = false;

  WiFiClient _client;

  if(_client.connect(target_ip, SERVER_PORT))
  {
    // send message
    _send_msg.set_msg(id);

    Serial.print("sending message to ");
    Serial.print(target_ip);
    Serial.println(":");
    _send_msg.print();

    _client.write(_send_msg.raw_data(),sizeof(_send_msg));


    if(MSG_ID_ACK != id)
    {
      // wait for response
      int rb = _client.readBytes(_rcv_msg.rcv_data(), sizeof(_rcv_msg));

      Serial.printf("received response: %d\n", rb);
      _rcv_msg.print();

      success = handle_ack_message();
    }
    else
    {
      success = true;
    }

    _client.flush();
    _client.stop();
  }

  return success;
}



////////////////////////////////////////
// ShootNet_Unit::check_for_message() //
////////////////////////////////////////
void ShootNet_Unit::check_for_message()
{
  WiFiClient _client = _server.available();
  if(_client)
  {
    if(_client.connected())
    {
      IPAddress client_ip = _client.remoteIP();

      _client.readBytes(_rcv_msg.rcv_data(), sizeof(_rcv_msg));

      Serial.print("received message from ");
      Serial.print(client_ip);
      Serial.println(":");
      _rcv_msg.print();

      if(validate_rcv_msg(client_ip))
      {
        Serial.print("sending ACK to ");
        Serial.println(client_ip);
        _send_msg.set_msg(MSG_ID_ACK);
        _client.write(_send_msg.raw_data(),sizeof(_send_msg));

        handle_rcv_messsage(client_ip);
      }
      else
      {
        Serial.println(" ->INVALID MESSAGE<-");
      }
    }
    _client.flush();
    _client.stop();
  }
}


/////////////////////////////////////////
// ShootNet_Unit::handle_ack_message() //
/////////////////////////////////////////
bool ShootNet_Unit::handle_ack_message()
{
  return _rcv_msg.get_id() == MSG_ID_ACK;
}


////////////////////////////////
// ShootNet_Unit::heartbeat() //
////////////////////////////////
void ShootNet_Unit::heartbeat()
{
  _last_heartbeat = _t_now;

  check_connection();
}


///////////////////////////////////////
// ShootNet_Unit::check_connection() //
///////////////////////////////////////
void ShootNet_Unit::check_connection()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED_BLUE_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_BLUE_PIN, LOW);
  }
}