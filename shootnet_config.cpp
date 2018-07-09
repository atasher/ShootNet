#include <shootnet_config.h>
#include <IPAddress.h>

// The SSID (name) of the Wi-Fi network you want to connect to
const char* ssid     = "your_ssid";

// The password of the Wi-Fi network (must be >= 8 characters for WPA2)
const char* password = "your_pw";

// The fixed IP address of the controller
// Only the controller needs a fixed IP since all communications go through it
//  (clients never communicate directly to one another)
const IPAddress CONTROLLER_IP(192,168,0,150);
