#ifndef FILE_CONFIGS
#define FILE_CONFIGS


// Replace the next variables with your SSID/Password combination
const char *ssid = "redmi";
const char *password = "123987abc";

//const char *ssid2 = "redmi_note_7";
//const char *password2 = "123987abc";



const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

///=============================================================

//#define MQTT_SECURE

// Add your MQTT Broker IP address, example:
// const char* mqtt_server = "192.168.1.144";
//const char *mqtt_server = "192.168.43.35";
const char *mqtt_server = "test.mosquitto.org";
//const char *mqtt_server = "192.168.0.4";

#ifdef MQTT_SECURE
const char *mqtt_username = "rcv_1";
const char *mqtt_password = "mqttreceiver1"; 
#endif




#endif