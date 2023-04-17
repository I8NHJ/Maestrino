const char CODE_VERSION[5] = "V2.2";
const char CODE_DATE[11] = "2023.04.17";
const char CALL_SIGN[6] = "N5NHJ";

#define ENCODERS 4  // Define the number of encoder in your project, 1 to 4

#define DEBUG

#define MEGA
// #define LEONARDO_ETH
// #define DUE

// #define DISPLAY_OLED091_I2C //0.91 OLED I2C Display Module IIC 0.91 128x32 inch I2C SSD1306 LED DC Display Module Blue I2C LCD 128x32 Screen Driver Compatible with OLED 3.3V~5V
 #define DISPLAY_OLED130_I2C  //HiLetgo 1.3" IIC I2C Serial 128x64 SSH1106 SSD1306 OLED LCD Display LCD Module for Arduino AVR PIC STM32

// Generic libraries
#include <SPI.h>
#include <Wire.h>
#include <Ethernet.h>
#include <Adafruit_GFX.h>

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <BfButton.h>
//#include <NSEncoder.h>

#include "FeaturesAndPinConfig.h"
#include "EthConfig.h"

// OLED Display set-up
// The pins for I2C are defined by the Wire-library.
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO_ETH:
// On an arduino DUE:
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

enum attribute {NoshowAttribute, showAttribute};
enum displayLines {firstLine, secondLine, thirdLine, forthLine};
enum messageType {standardMsg,toggleMsg};
const byte smallText = 1;
const byte medText = 2;
const byte bigText = 3;

EthernetClient client;  // Initialize the Ethernet client library

// Create Encoders,  Encoder Buttons, and define push buttons status

BfButton enc0Btn(BfButton::STANDALONE_DIGITAL, enc0BtnPin, true, LOW);  // 1 encoder. First Encoder is #0
Encoder enc0(ENCODER_0_S1_PIN, ENCODER_0_S2_PIN);

#if (ENCODERS > 1)
  BfButton enc1Btn(BfButton::STANDALONE_DIGITAL, enc1BtnPin, true, LOW);  // 2 encoders
  Encoder enc1(ENCODER_1_S1_PIN, ENCODER_1_S2_PIN);
#endif
#if (ENCODERS > 2)
  BfButton enc2Btn(BfButton::STANDALONE_DIGITAL, enc2BtnPin, true, LOW);  // 3 encoders
  Encoder enc2(ENCODER_2_S1_PIN, ENCODER_2_S2_PIN);
#endif
#if (ENCODERS > 3)
  BfButton enc3Btn(BfButton::STANDALONE_DIGITAL, enc3BtnPin, true, LOW);  // 4 encoders
  Encoder enc3(ENCODER_3_S1_PIN, ENCODER_3_S2_PIN);
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
  const byte msgIndexTable[3] = { byte(11), byte(12), byte(13) };
  byte encBtnStatus [1] = {1};
  #elif (ENCODERS == 2)
    const byte msgIndexTable[6] = { byte(11), byte(12), byte(13), byte(21), byte(22), byte(23) };
    byte encBtnStatus [2] = {1, 1};
  #elif (ENCODERS == 3)
    const byte msgIndexTable[9]  = { byte(11), byte(12), byte(13), byte(21), byte(22), byte(23), byte(31), byte(32), byte(33) };
    byte encBtnStatus [3] = {1, 1, 1};
  #elif (ENCODERS == 4)
    const byte msgIndexTable[12] = { byte(11), byte(12), byte(13), byte(21), byte(22), byte(23), byte(31), byte(32), byte(33), byte(41), byte(42), byte(43) };
    byte encBtnStatus [4] = {1, 1, 1, 1};
#endif

char showBuffer[31];
char httpHost[22]="Host: ";

int counter = 0;  
int encValue = 0;
int oldcounter[4] = {0,0,0,0};
byte encoderNumber = 0;

#ifdef DISPLAY_OLED091_I2C
  Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Define and create the SSD1306 display connected to I2C (SDA, SCL pins)
#endif

#ifdef DISPLAY_OLED130_I2C
  Adafruit_SH1106G oled = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // Define and create the SSH11X display connected to I2C (SDA, SCL pins)
#endif

void setup() {
  #if defined DEBUG && !defined LEONARDO_ETH
      Serial.begin(9600);
      Serial.println(F("\r\n---"));
  #endif

  #include "FRStackAPI.h"

  for (byte i = 0; i < ArraysSize; i++) { FRStackLastValue[i] = FRStackCmdDefaultValue[i]; }  // Init actual values from default

  strcat(httpHost, (String(FRStackPC[0]) + "." + String(FRStackPC[1]) + "." + String(FRStackPC[2]) + "." + String(FRStackPC[3])).c_str());
  #if defined DEBUG && !defined LEONARDO_ETH
    Serial.print(httpHost);
    Serial.print(F("\t lenght:"));
    Serial.println(String(strlen(httpHost)));     
  #endif

  #ifdef DISPLAY_OLED091_I2C
    if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      #if defined DEBUG && !defined LEONARDO_ETH
        Serial.println(F("SSD1306 allocation failed"));
      #endif
    for (;;)
    ;  // Don't proceed, loop forever
    }
    else
    {
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.println(F("OLED091 Initiated"));
    #endif
    }
  #endif //DISPLAY_OLED091_I2C

  #ifdef DISPLAY_OLED130_I2C
    if (!oled.begin(SCREEN_ADDRESS, false)) {
      #if defined DEBUG && !defined LEONARDO_ETH
        Serial.println(F("SH1106 allocation failed"));
      #endif
      for (;;)
      ;  // Don't proceed, loop forever
    }
    else
    {
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.println(F("OLED130 Initiated"));
    #endif
    }
  #endif //DISPLAY_OLED130_I2C

  oled.setTextColor(1, 0);

  Ethernet.init(EthCSPin);

  #ifndef LEONARDO_ETH
    oledShow("INIT ETH", 0, smallText, firstLine, NoshowAttribute);
  #endif

  // start the Ethernet connection:
  #ifdef STATIC_IP
   Ethernet.begin(mac, myIP, myDns, myGateway, mySubnet);     // Configure using IP address defined in EthConfig.h 
  #else
    if (Ethernet.begin(mac) == 0) {
      #if defined DEBUG && !defined LEONARDO_ETH
        Serial.println(F("Failed to configure Ethernet using DHCP"));
      #endif
    }
  #endif //STATIC-IP
  // Check for Ethernet Status 
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.println(F("Ethernet shield was not found"));
    #endif
    while (true) {
      delay(1);  // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.println(F("Ethernet cable is not connected."));
    #endif
  }
  #if defined DEBUG && !defined LEONARDO_ETH
    Serial.println(Ethernet.localIP());
  #endif

  IPAddress ip = Ethernet.localIP();

  #ifndef LEONARDO_ETH
    strcpy(showBuffer, (String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3])).c_str());
    oledShow(showBuffer,0, smallText, firstLine, NoshowAttribute);
  #endif

  delay(1000);  // give the Ethernet shield a second to initialize:
  
  #ifndef LEONARDO_ETH
    oledShow("connecting FRStackPC", 0, smallText, firstLine, NoshowAttribute);
    delay(1000);  //give the user some time to look at the display
  #endif

  if (client.connect(FRStackPC, FRStackPort)) {
    #ifndef LEONARDO_ETH   // if you get a connection, report back via OLED:
      oledShow(httpHost, 0, smallText, firstLine, NoshowAttribute);
      delay(500);
      // Init some FRStack parameters to their default values
      oledShow("Set default values", 0, smallText, firstLine, NoshowAttribute);
    #endif
    sendFRStackMsg(standardMsg, 0, FRStackCmdDefaultValue[0]);  //Volume
    delay(500);
    sendFRStackMsg(standardMsg, 3, FRStackCmdDefaultValue[3]);  //AGCT
    delay(500);
  }
  else {
    #ifndef LEONARDO_ETH
      oledShow("Connection failed", 0, smallText, firstLine, NoshowAttribute);
      delay(5000);
    #endif
  }
  strcpy (showBuffer, CALL_SIGN);
  strcat (showBuffer, " ");
  strcat (showBuffer, CODE_VERSION);
  strcat (showBuffer, " ");
  strcat (showBuffer, CODE_DATE);
  oledShow(showBuffer, 0, smallText, firstLine, NoshowAttribute);
  
  //Buttons handlers
  #if defined DEBUG && !defined LEONARDO_ETH
    Serial.println(F("ButtonHandler0 created"));
  #endif
  enc0Btn.onPress(pressHandler0)
    .onDoublePress(pressHandler0)      // default timeout
    .onPressFor(pressHandler0, 1000);  // custom timeout for 1 second

  #if (ENCODERS > 1) // 2,3, or 4 enocoders
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.println(F("ButtonHandler1 created"));
    #endif
    enc1Btn.onPress(pressHandler1)
      .onDoublePress(pressHandler1)      // default timeout
      .onPressFor(pressHandler1, 1000);  // custom timeout for 1 second
  #endif // ENCODERS > 1
  #if (ENCODERS > 2) // 3 or 4 encoders
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.println(F("ButtonHandler2 created"));
    #endif
    enc2Btn.onPress(pressHandler2)
      .onDoublePress(pressHandler2)      // default timeout
      .onPressFor(pressHandler2, 1000);  // custom timeout for 1 second
  #endif // ENCODERS > 2
  #if (ENCODERS > 3) // 4 encoders
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.println(F("ButtonHandler3 created"));
    #endif
    enc3Btn.onPress(pressHandler3)
      .onDoublePress(pressHandler3)      // default timeout
      .onPressFor(pressHandler3, 1000);  // custom timeout for 1 second
  #endif // ENCODERS > 3
}
// End setup()

void loop() {
  // Check on Encoder 0
  encoderNumber = 0;
  //Check for button 0 press
  enc0Btn.read();
  // Check for encoder 0 movement
  counter = enc0.read()/ENCODERS_STEPS;
  if (counter - oldcounter[encoderNumber]) {
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.print(F("diff enc 0: "));
      Serial.println(counter - oldcounter[encoderNumber]);
    #endif
    updateValue(encoderNumber, searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]), counter);
  }

  #if (ENCODERS > 1)
    // Check on Encoder 1
    encoderNumber = 1;
    //Check for button 1 press
    enc1Btn.read();
    // Check for encoder 1 movement
    counter = enc1.read()/ENCODERS_STEPS;
    if (counter - oldcounter[encoderNumber]) {
      #if defined DEBUG && !defined LEONARDO_ETH
        Serial.print(F("diff enc 1: "));
        Serial.println(counter - oldcounter[encoderNumber]);
      #endif
    updateValue(encoderNumber, searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]), counter);
    }
  #endif // 2 Encoders

  #if (ENCODERS > 2)
    // Check on Encoder 2
    encoderNumber = 2;
    //Check for button 2 press
    enc2Btn.read();
    // Check for encoder 2 movement
    counter = enc2.read()/ENCODERS_STEPS;
    if (counter - oldcounter[encoderNumber]) {
      #if defined DEBUG && !defined LEONARDO_ETH
        Serial.print(F("diff enc 2: "));
        Serial.println(counter - oldcounter[encoderNumber]);
      #endif
    updateValue(encoderNumber, searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]), counter);
    }
  #endif // 3 Encoders

  #if (ENCODERS > 3)
    // Check on Encoder 0
    encoderNumber = 3;
    //Check for button 3 press
    enc3Btn.read();
    // Check for encoder 3 movement
    counter = enc3.read()/ENCODERS_STEPS;
    if (counter - oldcounter[encoderNumber]) {
      #if defined DEBUG && !defined LEONARDO_ETH
        Serial.print(F("diff enc 3: "));
        Serial.println(counter - oldcounter[encoderNumber]);
      #endif
    updateValue(encoderNumber, searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]), counter);
    }
  #endif // 4 Encoders
}
// End loop()

// ------------------- FUNCTIONS ---------------------
// If encoder moved, update Value for the function, send and display 
void updateValue(byte encoderNumber, byte msgIndex, int counter) {
  encValue = FRStackLastValue[msgIndex] + ((counter - oldcounter[encoderNumber]) * FRStackCmdSteps[msgIndex]);
  if (encValue >= FRStackCmdHighLimit[msgIndex]) {
    encValue = FRStackCmdHighLimit[msgIndex];
  }
  if (encValue <= FRStackCmdLowLimit[msgIndex]) {
    encValue = FRStackCmdLowLimit[msgIndex];
  }
  sendFRStackMsg(standardMsg, msgIndex, encValue);
  oledShow(FRStackCmdLabel[msgIndex], encValue, medText, firstLine, showAttribute);
  FRStackLastValue[msgIndex] = encValue;
  oldcounter[encoderNumber] = counter;
} //End updateValue, 

// Search Message index from Encoder, Push Status
byte searchMesageIndex(byte EncoderID, byte EncoderPush) {
  for (byte i = 0; i < ArraysSize; i++) {
    #if defined DEBUG && !defined LEONARDO_ETH
      Serial.print (F("Loop Index: "));
      Serial.print (String(i));
      Serial.print (F("  Value to search:"));
      Serial.print (String(byte((EncoderID * 10 + EncoderPush))));
      Serial.print (" Value in the array:");
      Serial.println (String(msgIndexTable[i]));
    #endif        
    if (byte(((EncoderID + 1) * 10 + EncoderPush)) == msgIndexTable[i]) {
      return i;
    }
  }
  #if defined DEBUG && !defined LEONARDO_ETH
    Serial.print (String(EncoderID));
    Serial.print(F("\t"));
    Serial.print (String(EncoderPush));
    Serial.print(F("\t"));
    Serial.println(F("Message index error"));
  #endif
  return 0;
}  //End searchMessageIndex

// FRStack API Call
void sendFRStackMsg(messageType messageType, byte messageIndex, int functionValue) {
  char clientMsg [54] = "GET "; // make sure the FRStack API message doesn't exceed 40 characters oterwise chamge this accordingly
  if (messageType == standardMsg)  //Standard message
    {
      strcat (clientMsg, FRStackCmdString[messageIndex]);
      strcat (clientMsg, String(functionValue).c_str());
    }
  if (messageType == toggleMsg) //Togle message
   {           
     strcat (clientMsg, FRStackCmdTogleString[messageIndex]);
     if (functionValue != -32768) {  // Togle message with number as parameter
       strcat (clientMsg, String(functionValue).c_str());
      }
  }
  strcat (clientMsg, " HTTP/1.1");
  client.connect(FRStackPC, FRStackPort);
  client.println(clientMsg);
  client.println(httpHost);
  client.println(F("Connection: close"));
  client.println();
  client.flush();
  #if defined DEBUG && !defined LEONARDO_ETH
   Serial.println(clientMsg);
  #endif
}  //End sendFRStackMsg

//Display Functions on OLED
void oledShow(const char *messageText, int functionValue, byte textSize, displayLines lineNumber, attribute showAttribute) {
  char messageDisplay[30];
  #ifdef DISPLAY_OLED091_I2C
    int CursorY = 0;
  #endif
  #ifdef DISPLAY_OLED130_I2C
    int CursorY = 4;
  #endif
  oled.clearDisplay();  // Always do this everytime there is something to show
  oled.setCursor(11, CursorY);
  oled.setTextSize(2);
  oled.print(F("MAESTRINO"));
  strcpy(messageDisplay, messageText);
  if (functionValue == -32768) {
    strcat(messageDisplay, " Tog");
  }
  if (showAttribute) {
    strcat(messageDisplay, " ");
    strcat(messageDisplay, String(functionValue).c_str());
  }
  int CursorX = (128 - (strlen(messageDisplay) * 6 * textSize)) / 2;
  if (CursorX < 0) { CursorX = 0; }
  #if defined DEBUG && !defined LEONARDO_ETH
    Serial.print (messageDisplay);
    Serial.print(F("\t lenght: "));
    Serial.print (String(strlen(messageDisplay)));
    Serial.print(F("\t CursorX: "));
    Serial.println(String(CursorX));
  #endif
  #ifdef DISPLAY_OLED091_I2C
    if (textSize == 1) { CursorY = 20; }
    if (textSize == 2) { CursorY = 16; }
    if (textSize == 3) { CursorY = 0; }
  #endif
  #ifdef DISPLAY_OLED130_I2C
    if (textSize == 1) { CursorY = 36; }
    if (textSize == 2) { CursorY = 30; }
    if (textSize == 3) { CursorY = 20; }
  #endif
  oled.setTextSize(textSize);
  oled.setCursor(CursorX, CursorY);
  oled.print(messageDisplay);
  oled.display();
}  //End oledShow

//Button press hanlding function
void pressHandler0(BfButton *enc0Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
      if (encBtnStatus[encoderNumber] == 3) {
        encBtnStatus[encoderNumber] = 1;
      } else {
        encBtnStatus[encoderNumber]++;
      }
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(toggleMsg, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, 2, firstLine, NoshowAttribute);
      break;

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(standardMsg, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;
  }
}  //End pressHandler1

#if ENCODERS > 1
void pressHandler1(BfButton *enc1Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
      if (encBtnStatus[encoderNumber] == 3) {
        encBtnStatus[encoderNumber] = 1;
      } else {
        encBtnStatus[encoderNumber]++;
      }
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(toggleMsg, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, NoshowAttribute);
      break;

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(standardMsg, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;
  }
} 
#endif  //End pressHandler1

#if ENCODERS > 2
void pressHandler2(BfButton *enc2Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
      if (encBtnStatus[encoderNumber] == 3) {
        encBtnStatus[encoderNumber] = 1;
      } else {
        encBtnStatus[encoderNumber]++;
      }
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(standardMsg, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, NoshowAttribute);
      break;

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(standardMsg, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;
  }
} 
#endif  //End pressHandler2

#if ENCODERS > 3
void pressHandler3(BfButton *enc3Btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:  // Move to the next function
      if (encBtnStatus [encoderNumber] == 3) {
        encBtnStatus [encoderNumber]= 1;
      } else {
        encBtnStatus[encoderNumber]++;
      }
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;

    case BfButton::DOUBLE_PRESS:  // If managed, turn function On or Off
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(toggleMsg, msgIndex, FRStackCmdTogleParameter[msgIndex]);
      oledShow(FRStackCmdLabel[msgIndex], -32768, medText, firstLine, NoshowAttribute);
      break;

    case BfButton::LONG_PRESS:  // Send function default value
      msgIndex = searchMesageIndex(encoderNumber, encBtnStatus[encoderNumber]);
      sendFRStackMsg(standardMsg, msgIndex, FRStackCmdDefaultValue[msgIndex]);
      FRStackLastValue[msgIndex] = FRStackCmdDefaultValue[msgIndex];
      oledShow(FRStackCmdLabel[msgIndex], FRStackLastValue[msgIndex], medText, firstLine, showAttribute);
      break;
  }
}  
#endif //End pressHandler3
