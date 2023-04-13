// ETHERNET set-up
// #define MAX_SOCK_NUM 1

#define STATIC_IP // Comment this line to get DHCP 

IPAddress FRStackPC(192, 168, 1, 139);                //IP address of the FRStack PC
unsigned int FRStackPort = 13522;                     //Listening port for FRStack

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x98, 0x37 };  // Your Eth board MAC address

#ifdef STATIC_IP
  IPAddress myIP(192, 168, 1, 237);
  IPAddress myDns(192, 168, 1, 254);
  IPAddress myGateway(192, 168, 1, 254);
  IPAddress mySubnet(255,255,255,0);
#endif
