const char CODE_VERSION[] = "V2.0";
const char CODE_DATE[] = "2023.04.10";
const char CALL_SIGN [] = "N5NHJ";

#define ENCODERS 4  // Define the number of encoder in your project, 1 to 4

#define MEGA
// #define DUE
// #define LEONARDO_ETH

// #define DEBUG

// #define DISPLAY_OLED091_I2C //0.91 OLED I2C Display Module IIC 0.91 128x32 inch I2C SSD1306 LED DC Display Module Blue I2C LCD 128x32 Screen Driver Compatible with OLED 3.3V~5V
#define DISPLAY_OLED130_I2C  //HiLetgo 1.3" IIC I2C Serial 128x64 SSH1106 SSD1306 OLED LCD Display LCD Module for Arduino AVR PIC STM32

// Generic libraries
#include <SPI.h>
#include <Wire.h>
#include <Ethernet.h>
#include <Adafruit_GFX.h>

#include <BfButton.h>
#include <Encoder.h>
//#include <NSEncoder.h>

#include "FeaturesAndPinConfig.h"
#include "EthConfig.h"

// OLED Display set-up
// The pins for I2C are defined by the Wire-library.
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino DUE:
// On an arduino LEONARDO:
#ifdef DISPLAY_OLED091_I2C
  #include <Adafruit_SSD1306.h>  // Declaration and libraries for an SSD1306 display connected to I2C (SDA, SCL pins)
  #define SCREEN_WIDTH 128       // OLED display width, in pixels
  #define SCREEN_HEIGHT 32       // OLED display height, in pixels
  #define OLED_RESET -1          // Reset pin # (or -1 if sharing Arduino reset pin)
  #define SCREEN_ADDRESS 0x3C    // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
  #define SSD1306_NO_SPLASH
#endif // DISPLAY_OLED091_I2C

#ifdef DISPLAY_OLED130_I2C
  #include <Adafruit_SH110X.h>  // Declaration for an SSH1106 display connected to I2C (SDA, SCL pins)
  #define SCREEN_WIDTH 128      // OLED display width, in pixels
  #define SCREEN_HEIGHT 64      // OLED display height, in pixels
  #define OLED_RESET -1         // Reset pin # (or -1 if sharing Arduino reset pin)
  #define SCREEN_ADDRESS 0x3c   //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
  #define SH110X_NO_SPLASH
#endif // DISPLAY_OLED130_I2C

enum showAttribute {showAttributeTrue, showAttributeFalse};
enum displayLines {firstLine, secondLine,thirdLine, forthLine};
enum textSize {smallText, medText, bigText};

EthernetClient client;  // Initialize the Ethernet client library

// Create Encoders,  Encoder Buttons, and define push buttons status
BfButton enc1Btn(BfButton::STANDALONE_DIGITAL, enc1BtnPin, true, LOW);  // 1 encoder
Encoder enc1(ENCODER_1_S1_PIN, ENCODER_1_S2_PIN);
// NSEncoder enc1(ENCODER_1_S1_PIN, ENCODER_1_S2_PIN, ENCODERS_STEPS);
byte enc1BtnStatus = 1;
#if (ENCODERS > 1)
  BfButton enc2Btn(BfButton::STANDALONE_DIGITAL, enc2BtnPin, true, LOW);  // 2 encoders
  Encoder enc2(ENCODER_2_S1_PIN, ENCODER_2_S2_PIN);
  //  NSEncoder enc2(ENCODER_2_S1_PIN, ENCODER_2_S2_PIN, ENCODERS_STEPS);
  byte enc2BtnStatus = 1;
#endif
#if (ENCODERS > 2)
  BfButton enc3Btn(BfButton::STANDALONE_DIGITAL, enc3BtnPin, true, LOW);  // 3 encoders
  Encoder enc3(ENCODER_3_S1_PIN, ENCODER_3_S2_PIN);
  //  NSEncoder enc3(ENCODER_3_S1_PIN, ENCODER_3_S2_PIN, ENCODERS_STEPS);
  byte enc3BtnStatus = 1;
#endif
#if (ENCODERS > 3)
  BfButton enc4Btn(BfButton::STANDALONE_DIGITAL, enc4BtnPin, true, LOW);  // 4 encoders
  Encoder enc4(ENCODER_4_S1_PIN, ENCODER_4_S2_PIN);
  //  NSEncoder enc4(ENCODER_4_S1_PIN, ENCODER_4_S2_PIN, ENCODERS_STEPS);
  byte enc4BtnStatus = 1;
#endif

const byte ArraysSize = ENCODERS * 3;  // FRStack arrays size = Number of encoders for 3 functions each

// Define FRStack command strings for buttons and encoders
const char *FRStackCmdString[ArraysSize];
int FRStackCmdDefaultValue[ArraysSize];
int FRStackCmdLowLimit[ArraysSize];
int FRStackCmdHighLimit[ArraysSize];
byte FRStackCmdSteps[ArraysSize];
const char *FRStackCmdLabel[ArraysSize];
const char *FRStackCmdTogleString[ArraysSize];
unsigned int FRStackCmdTogleParameter[ArraysSize];
int FRStackLastValue[ArraysSize];

byte msgIndex = 0;
#if (ENCODERS == 1)
const byte msgIndexTable[3] PROGMEM = { byte(11), byte(12), byte(13) };
#elif (ENCODERS == 2)
const byte msgIndexTable[6] PROGMEM = { 11, 12, 13, 21, 22, 23 };
#elif (ENCODERS == 3)
const byte msgIndexTable[9] PROGMEM = { 11, 12, 13, 21, 22, 23, 31, 32, 33 };
#else
const byte msgIndexTable[12] = { byte(11), byte(12), byte(13), byte(21), byte(22), byte(23), byte(31), byte(32), byte(33), byte(41), byte(42), byte(43) };
#endif

const String httpPre = "GET ";
const String httpPos = " HTTP/1.1";
char httpHost[] = "Host: www.xxx.yyy.zzz";
String FRStackMsg = "";

int counter = 0;  // can those be byte?
int encCounter = 0;
int encValue = 0;
bool encChanged = false;

#ifdef DISPLAY_OLED091_I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Define and create the SSD1306 display connected to I2C (SDA, SCL pins)
#endif

#ifdef DISPLAY_OLED130_I2C
Adafruit_SH1106G oled = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Define and create the SSH11X display connected to I2C (SDA, SCL pins)
#endif

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
#endif

#include "FRStackAPI.h"

for (byte i = 0; i < ArraysSize; i++) { FRStackLastValue[i] = FRStackCmdDefaultValue[i]; }  // Init actual values from default

strcpy(httpHost, "Host: ");  //To be tested !
strcat(httpHost, (String(FRStackPC[0]) + "." + String(FRStackPC[1]) + "." + String(FRStackPC[2]) + "." + String(FRStackPC[3])).c_str());

#ifdef DISPLAY_OLED091_I2C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
#ifdef DEBUG
    Serial.println(F("SSD1306 allocation failed"));
#endif
    for (;;)
      ;  // Don't proceed, loop forever
  } else {
#ifdef DEBUG
    Serial.println(F("OLED Initiated"));
#endif
  }
#endif //DISPLAY_OLED091_I2C

#ifdef DISPLAY_OLED130_I2C
  if (!oled.begin(SCREEN_ADDRESS, false)) {
#ifdef DEBUG
    Serial.println(F("SH1106 allocation failed"));
#endif
    for (;;)
      ;  // Don't proceed, loop forever
  } else {
#ifdef DEBUG
    Serial.println(F("OLED Initiated"));
#endif
  }
#endif //DISPLAY_OLED130_I2C

  oled.setTextColor(1, 0);

  Ethernet.init(EthCSPin);

  oledShow((const char*) F("INIT ETH"), 0, smallText, firstLine, showAttributeFalse);

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
    Ethernet.begin(mac, myIP, myDns, myGateway);
  } else {
    oledShow((String(myIP[0]) + "." + String(myIP[1]) + "." + String(myIP[2]) + "." + String(myIP[3])).c_str(), 0, smallText, firstLine, showAttributeFalse);
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
  oledShow((const char*) F("connecting FRStackPC"), 0, smallText, firstLine, showAttributeFalse);
  delay(1000);  //give some time to look at the display actions

  // if you get a connection, report back via OLED:
  if (client.connect(FRStackPC, FRStackPort)) {
    oledShow(httpHost, 0, smallText, firstLine, showAttributeFalse);
    delay(500);
    // Init some FRStack parameters to their default values
    oledShow((const char*) F("Set default values"), 0, smallText, firstLine, showAttributeFalse);
    sendFRStackMsg(0, 0, FRStackCmdDefaultValue[0]);  //Volume
    delay(500);
    sendFRStackMsg(0, 3, FRStackCmdDefaultValue[3]);  //AGCT
    delay(500);
  } else {
    // if you didn't get a connection to the server:
    oledShow((const char*) F("Connection failed"), 0, smallText, firstLine, showAttributeFalse);
    delay(5000);
  }
  oledShow(buildBanner(), 0, smallText, firstLine, showAttributeFalse);
  //oledShow((String(CALL_SIGN) + " " + String(CODE_VERSION) + " " + String(CODE_DATE)).c_str(), 0, smallText, firstLine, showAttributeFalse);
  //Buttons handlers
  enc1Btn.onPress(pressHandler1)
    .onDoublePress(pressHandler1)      // default timeout
    .onPressFor(pressHandler1, 1000);  // custom timeout for 1 second
#if (ENCODERS > 1)
  enc2Btn.onPress(pressHandler2)
    .onDoublePress(pressHandler2)      // default timeout
    .onPressFor(pressHandler2, 1000);  // custom timeout for 1 second
#endif // ENCODERS > 1
#if (ENCODERS > 2)
  enc3Btn.onPress(pressHandler3)
    .onDoublePress(pressHandler3)      // default timeout
    .onPressFor(pressHandler3, 1000);  // custom timeout for 1 second
#endif // ENCODERS > 2
#if (ENCODERS > 3)
  enc4Btn.onPress(pressHandler4)
    .onDoublePress(pressHandler4)      // default timeout
    .onPressFor(pressHandler4, 1000);  // custom timeout for 1 second
#endif // ENCODERS > 3
}
// End setup()

void loop() {

  //Check for button press
  enc1Btn.read();
#if (ENCODERS > 1)
  enc2Btn.read();
#endif
#if (ENCODERS > 2)
  enc3Btn.read();
#endif
#if (ENCODERS > 3)
  enc4Btn.read();
#endif

  // Check for encoders movement
  encChanged = false;

  if ((counter = enc1.readAndReset()) != 0) {
    //if((counter = enc1.get_diffPosition()) != 0) {
    encCounter = counter / ENCODERS_STEPS;
    msgIndex = searchMesageIndex(1, enc1BtnStatus);
    encChanged = true;
  }

#if (ENCODERS > 1)
  if ((counter = enc2.readAndReset()) != 0) {
    //  if((counter = enc2.get_diffPosition()) != 0) {
    encCounter = counter;
    msgIndex = searchMesageIndex(2, enc2BtnStatus);
    encChanged = true;
  }
#endif

#if (ENCODERS > 2)
  if ((counter = enc3.readAndReset()) != 0) {
    //  if((counter = enc3.get_diffPosition()) != 0) {
    encCounter = counter / ENCODERS_STEPS;
    msgIndex = searchMesageIndex(3, enc3BtnStatus);
    encChanged = true;
  }
#endif

#if (ENCODERS > 3)
  if ((counter = enc4.readAndReset()) != 0) {
    //  if((counter = enc4.get_diffPosition()) != 0) {
    encCounter = counter / ENCODERS_STEPS;
    msgIndex = searchMesageIndex(4, enc4BtnStatus);
    encChanged = true;
  }
#endif

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
// Build the banner as a pointer to chars
char * buildBanner () {
  static char TempString [30];
  strcpy (TempString, CALL_SIGN);
  strcat (TempString, (const char*)(" "));
  strcat (TempString, CODE_VERSION);
  strcat (TempString, (const char*)(" "));
  strcat (TempString, CODE_DATE);
  return TempString;
}
// Search Message index from Encoder, Push Status
byte searchMesageIndex(byte EncoderID, byte EncoderPush) {
  for (byte i = 0; i < ArraysSize; i++) {
    #ifdef DEBUG
      Serial.print (F("Loop Index: "));
      Serial.print (String(i));
      Serial.print (F("  Value to search:"));
      Serial.print (String(byte((EncoderID * 10 + EncoderPush))));
      Serial.print (" Value in the array:");
      Serial.println (String(msgIndexTable[i]));
    #endif        
    if (byte((EncoderID * 10 + EncoderPush)) == msgIndexTable[i]) {
      return i;
    }
  }
#ifdef DEBUG
  Serial.print (String(EncoderID));
  Serial.print(F("\t"));
  Serial.print (String(EncoderPush));
  Serial.print(F("\t"));
  Serial.println(F("Message index error"));
#endif
  return 0;
}  //End searchMessageIndex

// FRStack API Call
void sendFRStackMsg(byte messageType, byte messageIndex, int functionValue) {
  client.connect(FRStackPC, FRStackPort);
  char clientMsg [50] PROGMEM = "GET ";
  if (messageType == 0) {
    strcat (clientMsg, FRStackCmdString[messageIndex]); //Standard message
    //FRStackMsg = (httpPre + FRStackCmdString[messageIndex] + String(functionValue) + httpPos);
  }
  if (messageType == 1) {           //Togle message
    strcat (clientMsg, FRStackCmdTogleString[messageIndex]);
    if (functionValue != -32768) {  // Togle message with number as parameter
    strcat (clientMsg, String(functionValue).c_str());
   //   FRStackMsg = (httpPre + FRStackCmdTogleString[messageIndex] + String(functionValue) + httpPos);
    }
    // else {  //Togle message with string asd part of the message
    //    strcat (clientMsg, FRStackCmdString[messageIndex]);
    //  FRStackMsg = (httpPre + FRStackCmdTogleString[messageIndex] + httpPos);
    //}
  }
  strcat (clientMsg, " HTTP/1.1"); 
  client.println(clientMsg);
#ifdef DEBUG
  Serial.println(clientMsg);
#endif
  client.println(httpHost);
  client.println(F("Connection: close"));
  client.println();
  client.flush();
}  //End sendFRStackMsg

//Display Functions on OLED
//void oledShow(String messageText, int functionValue, int textSize, int lineNumber, bool showAttribute) {
void oledShow(const char *messageText, int functionValue, textSize textSize, displayLines lineNumber, showAttribute showAttribute) {
  oled.clearDisplay();  // Always do this everytime there is something to show
char messageDisplay [30] PROGMEM;
#ifdef DISPLAY_OLED091_I2C
  int CursorY = 0;
#endif
#ifdef DISPLAY_OLED130_I2C
  int CursorY = 4;
#endif
  oled.setCursor(11, CursorY);
  oled.setTextSize(2);
  oled.print(F("MAESTRINO"));
  strcpy_P(messageDisplay, messageText);
  if (functionValue == -32768) {
    strcat(messageDisplay, (const char*) F(" Tog"));
    //   messageText = messageText + " Tog";
  }
  if (showAttributeTrue) {
    strcat(messageDisplay, (const char*) F(" "));
    strcat(messageDisplay, String(functionValue).c_str());
    //  messageText = messageText + " " + String(functionValue);
  }

  int CursorX = (128 - (strlen(messageDisplay) * 6 * textSize)) / 2;
    //int CursorX = (128 - (messageText.length()*6*textSize)) / 2;

  if (CursorX < 0) { CursorX = 0; }
  oled.setTextSize(textSize);

#ifdef DISPLAY_OLED091_I2C
  if (textSize == smallText) { CursorY = 20; }
  if (textSize == medText) { CursorY = 16; }
  if (textSize == bigText) { CursorY = 0; }
#endif
#ifdef DISPLAY_OLED130_I2C
  if (textSize == smallText) { CursorY = 36; }
  if (textSize == medText) { CursorY = 30; }
  if (textSize == bigText) { CursorY = 20; }
#endif

  oled.setCursor(CursorX, CursorY);
  oled.print(messageDisplay);
  oled.display();
}  //End oledShow

//Button press hanlding function
void pressHandler1(BfButton *enc1Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
      if (enc1BtnStatus == 3) {
        enc1BtnStatus = 1;
      } else {
        enc1BtnStatus++;
      }
      msgIndex = searchMesageIndex(1, enc1BtnStatus);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(1, enc1BtnStatus);
      sendFRStackMsg(1, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, showAttributeFalse);
      break;

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(1, enc1BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;
  }
}  //End pressHandler1

#if ENCODERS > 1
void pressHandler2(BfButton *enc2Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
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

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(2, enc2BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;
  }
} 
#endif  //End pressHandler2

#if ENCODERS > 2
void pressHandler3(BfButton *enc3Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
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

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(3, enc3BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;
  }
}  //End pressHandler3
#endif

#if ENCODERS > 3
void pressHandler4(BfButton *enc4Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
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

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(4, enc4BtnStatus);
      sendFRStackMsg(0, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttributeTrue);
      break;
  }
}  //End pressHandler4
#endif
