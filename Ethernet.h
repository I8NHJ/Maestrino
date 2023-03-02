// ETHERNET set-up
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x98, 0x37 };  // Enter a MAC address for your controller.
IPAddress FRStackPC(192, 168, 1, 139);                // use the numeric IP instead of the name for the server:
IPAddress ip(192, 168, 1, 237);
IPAddress myDns(192, 168, 1, 254);
IPAddress gateway(192, 168, 1, 254);
int FRStackPort = 13522;
String myIP = "";