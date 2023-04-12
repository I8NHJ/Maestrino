// ETHERNET set-up
// #define MAX_SOCK_NUM 1
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x98, 0x37 };  // Enter a MAC address for your controller.
IPAddress FRStackPC(192, 168, 1, 139);                // use the numeric IP instead for the FRstack PC on your lan.
IPAddress myIP(192, 168, 1, 237);
IPAddress myDns(192, 168, 1, 254);
IPAddress myGateway(192, 168, 1, 254);
unsigned int FRStackPort = 13522;
