#ifndef WEBSERVER_ENV
#define WEBSERVER_ENV
/// WEB SERVER CONFIG
// Set web server port number to 80
WiFiServer localServer(80);
// Variable to store the HTTP request
String localWebpageHeader;
#endif