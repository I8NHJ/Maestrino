#define CODE_VERSION "V1.4"
#define CODE_DATE "2023.04.02"
#define CALL_SIGN "N5NHJ"
#define ENCODERS 4 // Define the number of encoder in your project, 1 to 4

// #define DEBUG

#define MEGA
// #define DUE

// #define DISPLAY_OLED091_I2C //0.91 OLED I2C Display Module IIC 0.91 128x32 inch I2C SSD1306 LED DC Display Module Blue I2C LCD 128x32 Screen Driver Compatible with OLED 3.3V~5V
#define DISPLAY_OLED130_I2C //HiLetgo 1.3" IIC I2C Serial 128x64 SSH1106 SSD1306 OLED LCD Display LCD Module for Arduino AVR PIC STM32

// Generic libraries
#include <SPI.h>
#include <Wire.h>
#include <Ethernet.h>
#include <Adafruit_GFX.h>

#include <BfButton.h>
#include <NSEncoder.h>

#include "FeaturesAndPinConfig.h"
#include "EthConfig.h"

// OLED Display set-up
// The pins for I2C are defined by the Wire-library.
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino DUE:
#ifdef DISPLAY_OLED091_I2C          
  #include <Adafruit_SSD1306.h>    // Declaration and libraries for an SSD1306 display connected to I2C (SDA, SCL pins)
  #define SCREEN_WIDTH 128         // OLED display width, in pixels
  #define SCREEN_HEIGHT 32         // OLED display height, in pixels
  #define OLED_RESET -1            // Reset pin # (or -1 if sharing Arduino reset pin)
  #define SCREEN_ADDRESS 0x3C      // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
  #define SSD1306_NO_SPLASH
#endif

#ifdef DISPLAY_OLED130_I2C
  #include <Adafruit_SH110X.h>     // Declaration for an SSH1106 display connected to I2C (SDA, SCL pins)
  #define SCREEN_WIDTH 128         // OLED display width, in pixels
  #define SCREEN_HEIGHT 64         // OLED display height, in pixels
  #define OLED_RESET -1            // Reset pin # (or -1 if sharing Arduino reset pin)
  #define SCREEN_ADDRESS 0x3c      //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
  #define SH110X_NO_SPLASH
#endif

const bool showAttributeTrue = true;
const bool showAttributeFalse = false;
const int firstLine = 1;
const int secondLine = 2;
const int thirdLine = 3;
const int forthLine = 4;
const int smallText = 1;
const int medText = 2;
const int bigText = 3;
String oledMsg = "";

EthernetClient client;             // Initialize the Ethernet client library

// Create Encoders,  Encoder Buttons, and define push buttons status
BfButton enc1Btn(BfButton::STANDALONE_DIGITAL, enc1BtnPin, true, LOW); // 1 encoder
NSEncoder enc1(ENCODER_1_S1_PIN, ENCODER_1_S2_PIN, ENCODERS_STEPS);
int enc1BtnStatus = 1;
#ifdef ENCODERS > 1 
  BfButton enc2Btn(BfButton::STANDALONE_DIGITAL, enc2BtnPin, true, LOW); // 2 encoders
  NSEncoder enc2(ENCODER_2_S1_PIN, ENCODER_2_S2_PIN, ENCODERS_STEPS);
  int enc2BtnStatus = 1;
#endif
#ifdef ENCODERS > 2
  BfButton enc3Btn(BfButton::STANDALONE_DIGITAL, enc3BtnPin, true, LOW); // 3 encoders
  NSEncoder enc3(ENCODER_3_S1_PIN, ENCODER_3_S2_PIN, ENCODERS_STEPS);
  int enc3BtnStatus = 1;
#endif
#ifdef ENCODERS > 3
  BfButton enc4Btn(BfButton::STANDALONE_DIGITAL, enc4BtnPin, true, LOW); // 4 encoders
  NSEncoder enc4(ENCODER_4_S1_PIN, ENCODER_4_S2_PIN, ENCODERS_STEPS);
  int enc4BtnStatus = 1;
#endif

const unsigned int ArraysSize = ENCODERS * 3; // FRStack arrays size = Number of encoders for 3 functions each

// Define FRStack command strings for buttons and encoders
char * FRStackCmdString[ArraysSize];
// String FRStackCmdString[ArraysSize];
int    FRStackCmdDefaultValue[ArraysSize];
int    FRStackCmdLowLimit[ArraysSize];
int    FRStackCmdHighLimit[ArraysSize];
int    FRStackCmdSteps[ArraysSize];
char * FRStackCmdLabel[ArraysSize];
char * FRStackCmdTogleString[ArraysSize];
int    FRStackCmdTogleParameter[ArraysSize];
int    FRStackLastValue[ArraysSize];

int msgIndex = 0;
const int msgIndexTable [12] PROGMEM = {11,12,13,21,22,23,31,32,33,41,42,43};

const String httpPre = "GET ";
const String httpPos = " HTTP/1.1";
String httpHost = "";
String FRStackMsg = "";

int counter = 0;
int encCounter = 0;
int encValue = 0;
bool encChanged = false;

#ifdef DISPLAY_OLED091_I2C
  Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   // Define and create the SSD1306 display connected to I2C (SDA, SCL pins)
#endif

#ifdef DISPLAY_OLED130_I2C
  Adafruit_SH1106G oled = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Define and create the SSH11X display connected to I2C (SDA, SCL pins)
#endif

void setup() {
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

 #include "FRStackAPI.h" 
  
  for (int i = 0; i < ArraysSize; i++) {FRStackLastValue[i] = FRStackCmdDefaultValue[i];}

  httpHost = "Host: " + String(FRStackPC[0]) + "." + String(FRStackPC[1]) + "." + String(FRStackPC[2]) + "." + String(FRStackPC[3]);
  
  #ifdef DISPLAY_OLED091_I2C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    #ifdef DEBUG
      Serial.println(F("SSD1306 allocation failed"));
    #endif
    for (;;);  // Don't proceed, loop forever
  } else {
    #ifdef DEBUG
      Serial.println(F("OLED Initiated"));
    #endif
  }
  #endif

  #ifdef DISPLAY_OLED130_I2C
  if (!oled.begin(SCREEN_ADDRESS, false)) {
    #ifdef DEBUG
      Serial.println(F("SH1106 allocation failed"));
    #endif
    for (;;);  // Don't proceed, loop forever
  } else {
    #ifdef DEBUG
      Serial.println(F("OLED Initiated"));
    #endif
  }
  #endif

  oled.setTextColor(1,0);

  Ethernet.init(EthCSPin);
  
  oledShow("INIT ETH", 0, smallText, firstLine, showAttributeFalse);

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    #ifdef DEBUG
      Serial.println(F("Failed to configure Ethernet using DHCP"));
    #endif
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      #ifdef DEBUG
        Serial.println(F("Ethernet shield was not found"));
      #endif
      while (true) {
        delay(1);  // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      #ifdef DEBUG
        Serial.println(F("Ethernet cable is not connected."));
      #endif
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns, gateway);
  } else {
    myIP = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    oledShow(myIP, 0, smallText, firstLine, showAttributeFalse);
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
  oledShow("connecting FRStackPC", 0, smallText, firstLine, showAttributeFalse);
  delay(1000);  //give some time to look at the display actions
  
  // if you get a connection, report back via OLED:
  if (client.connect(FRStackPC, FRStackPort)) {
    oledShow(httpHost, 0, smallText, firstLine, showAttributeFalse);
    delay (500);
    // Init some FRStack parameters to their default values
    oledShow("Set default values", 0, smallText, firstLine, showAttributeFalse);
    sendFRStackMsg(0, 0, FRStackCmdDefaultValue[0]); //Volume 
    delay (500);
    sendFRStackMsg(0, 3, FRStackCmdDefaultValue[3]); //AGCT 
    delay (500);
  } else {
    // if you didn't get a connection to the server:
    oledShow("Connection failed", 0, smallText, firstLine, showAttributeFalse);
    delay (5000);
  }
  oledShow(String(CALL_SIGN) + " " + String(CODE_VERSION) + " " + String(CODE_DATE), 0, smallText, firstLine, showAttributeFalse);

  //Buttons handlers
  enc1Btn.onPress(pressHandler1)
    .onDoublePress(pressHandler1)      // default timeout
    .onPressFor(pressHandler1, 1000);  // custom timeout for 1 second
  #ifdef ENCODERS > 1
  enc2Btn.onPress(pressHandler2)
    .onDoublePress(pressHandler2)      // default timeout
    .onPressFor(pressHandler2, 1000);  // custom timeout for 1 second
  #endif
  #ifdef ENCODERS > 2
  enc3Btn.onPress(pressHandler3)
    .onDoublePress(pressHandler3)      // default timeout
    .onPressFor(pressHandler3, 1000);  // custom timeout for 1 second
  #endif
  #ifdef ENCODERS > 3
  enc4Btn.onPress(pressHandler4)
    .onDoublePress(pressHandler4)      // default timeout
    .onPressFor(pressHandler4, 1000);  // custom timeout for 1 second
  #endif
}
// End setup()

void loop() {

  //Wait for button press to execute commands
  enc1Btn.read();
  #ifdef ENCODERS > 1
    enc2Btn.read();
  #endif
  #ifdef ENCODERS > 2
    enc3Btn.read();
  #endif
  #ifdef ENCODERS > 1
    enc4Btn.read();
  #endif

  // Check for encoders movement
  encChanged = false;
  for (int i = 1; i < (ENCODERS+1) ; i++) {
    switch (i) {
      case 1:
        if((counter = enc1.get_diffPosition()) != 0) {
          encCounter=counter;
          msgIndex = searchMesageIndex(1, enc1BtnStatus);
          encChanged = true;
        }
        break;
      case 2:
        if((counter = enc2.get_diffPosition()) != 0) {
          encCounter=counter;
          msgIndex = searchMesageIndex(2, enc2BtnStatus);
          encChanged = true;
        }
        break;
      case 3:
        if((counter = enc3.get_diffPosition()) != 0) {
          encCounter=counter;
          msgIndex = searchMesageIndex(3, enc3BtnStatus);
          encChanged = true;
        }
        break;
      case 4:
        if((counter = enc4.get_diffPosition()) != 0) {
          encCounter=counter;
          msgIndex = searchMesageIndex(4, enc4BtnStatus);
          encChanged = true;
        }
        break;
    }
  }

  if (encChanged) {
    encValue = FRStackLastValue[msgIndex] + (encCounter * FRStackCmdSteps[msgIndex]);
      if (encValue >= FRStackCmdHighLimit[msgIndex]) {
         encValue = FRStackCmdHighLimit[msgIndex];
      }
     if (encValue <= FRStackCmdLowLimit[msgIndex]) {
         encValue = FRStackCmdLowLimit[msgIndex];
      }
     sendFRStackMsg(0, msgIndex, encValue);
     oledShow(FRStackCmdLabel[msgIndex], encValue, medText, firstLine, showAttributeTrue);
     FRStackLastValue[msgIndex] = encValue;
  }
}

// ------------------- FUNCTIONS ---------------------

// Search Message index from Encoder, Push Status
int searchMesageIndex(int EncoderID, int EncoderPush) {
  for (int i = 0; i < ArraysSize; i++)
  {
    if ((EncoderID*10 + EncoderPush) == msgIndexTable[i]) {
      return i;
    }
  }
  #ifdef DEBUG
    Serial.println (F("Message index error"));
  #endif
  return 0;
}
//End searchMessageIndex

// FRStack API Call
void sendFRStackMsg (int messageType, int messageIndex, int functionValue) {
  client.connect(FRStackPC, FRStackPort);
  if (messageType == 0) { //Standard message
  FRStackMsg = (httpPre + FRStackCmdString[messageIndex] + String(functionValue) + httpPos);
  }
  if (messageType == 1) { //Togle message
    if (functionValue != -32768) { // Togle message with number as parameter
      FRStackMsg = (httpPre + FRStackCmdTogleString[messageIndex] + String(functionValue) + httpPos);
    }
    else { //Togle message with string asd part of the message
      FRStackMsg = (httpPre + FRStackCmdTogleString[messageIndex] + httpPos); 
    }
  }
  client.println(FRStackMsg);
  #ifdef DEBUG
    Serial.println(FRStackMsg);
  #endif
  client.println(httpHost);
  client.println(F("Connection: close"));
  client.println();
  client.flush();
}
//End sendFRStackMsg

//Display Functions on OLED
void oledShow(String messageText, int functionValue, int textSize, int lineNumber, bool showAttribute) {
  // Always do this everytime there is something to show
  oled.clearDisplay();

  #ifdef DISPLAY_OLED091_I2C  
    int CursorY = 0;
  #endif
  #ifdef DISPLAY_OLED130_I2C
    int CursorY = 4;
  #endif
  oled.setCursor(11, CursorY);
  oled.setTextSize(2);
  oled.print(F("MAESTRINO"));

  if (functionValue == -32768) {
      messageText = messageText + " Tog";
    }
  if (showAttribute)
    {
      messageText = messageText + " " + String(functionValue);
    }

  int CursorX = (128 - (messageText.length()*6*textSize)) / 2;
  if (CursorX < 0) {CursorX = 0;}
  oled.setTextSize(textSize);

  #ifdef DISPLAY_OLED091_I2C  
    if (textSize == smallText) {CursorY = 20;}
    if (textSize == medText)   {CursorY = 16;}
    if (textSize == bigText)   {CursorY = 0; }
  #endif
  #ifdef DISPLAY_OLED130_I2C 
    if (textSize == smallText) {CursorY = 36;}
    if (textSize == medText)   {CursorY = 30;}
    if (textSize == bigText)   {CursorY = 20;}
  #endif

  oled.setCursor(CursorX, CursorY);
  oled.print(messageText);
  oled.display();
}
//End oledShow

//Button press hanlding function
void pressHandler1(BfButton *enc1Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS: // Move to the next function
      if (enc1BtnStatus == 3) {
        enc1BtnStatus = 1;
      } else {
        enc1BtnStatus++;
      }
      msgIndex = searchMesageIndex(1, enc1BtnStatus);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;

    case BfButton::DOUBLE_PRESS: // If managed, turn function On or Off
      msgIndex = searchMesageIndex(1, enc1BtnStatus);
      sendFRStackMsg(1, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, showAttributeFalse);     
      break;

    case BfButton::LONG_PRESS: // Send function default value
      msgIndex = searchMesageIndex(1, enc1BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);      
      break;
  }
}
//End pressHandler1

void pressHandler2(BfButton *enc2Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS: // Move to the next function
      if (enc2BtnStatus == 3) {
        enc2BtnStatus = 1;
      } else {
        enc2BtnStatus++;
      }
      msgIndex = searchMesageIndex(2, enc2BtnStatus);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(2, enc2BtnStatus);
      sendFRStackMsg(1, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, showAttributeFalse);     
      break;

    case BfButton::LONG_PRESS: // Send function default value
      msgIndex = searchMesageIndex(2, enc2BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;
  }
}
//End pressHandler2

void pressHandler3(BfButton *enc3Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
     case BfButton::SINGLE_PRESS: // Move to the next function
      if (enc3BtnStatus == 3) {
        enc3BtnStatus = 1;
      } else {
        enc3BtnStatus++;
      }
      msgIndex = searchMesageIndex(3, enc3BtnStatus);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(3, enc3BtnStatus);
      sendFRStackMsg(1, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, showAttributeFalse);     
      break;

    case BfButton::LONG_PRESS: // Send function default value
      msgIndex = searchMesageIndex(3, enc3BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;
  }
}
//End pressHandler3

void pressHandler4(BfButton *enc4Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS: // Move to the next function
      if (enc4BtnStatus == 3) {
        enc4BtnStatus = 1;
      } else {
        enc4BtnStatus++;
      }
      msgIndex = searchMesageIndex(4, enc4BtnStatus);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(4, enc4BtnStatus);
      sendFRStackMsg(1, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, showAttributeFalse);     
      break;

    case BfButton::LONG_PRESS: // Send function default value
      msgIndex = searchMesageIndex(4, enc4BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;
  }
}
//End pressHandler4