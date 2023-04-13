const char CODE_VERSION[5] = "V2.1";
const char CODE_DATE[11] = "2023.04.13";
const char CALL_SIGN[6] = "N5NHJ";

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

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <BfButton.h>
//#include <NSEncoder.h>

#include "FeaturesAndPinConfig.h"
#include "EthConfig.h"

// OLED Display set-up
// The pins for I2C are defined by the Wire-library.
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino DUE:
// On an arduino LEONARDO_ETH:
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

enum showAttribute {showAttributeFalse, showAttributeTrue};
enum displayLines {firstLine, secondLine, thirdLine, forthLine};
const byte smallText = 1;
const byte medText = 2;
const byte bigText = 3;

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
  const byte msgIndexTable[3] = { byte(11), byte(12), byte(13) };
  #elif (ENCODERS == 2)
    const byte msgIndexTable[6] = { byte(11), byte(12), byte(13), byte(21), byte(22), byte(23) };
  #elif (ENCODERS == 3)
    const byte msgIndexTable[9]  = { byte(11), byte(12), byte(13), byte(21), byte(22), byte(23), byte(31), byte(32), byte(33) };
  #elif (ENCODERS == 4)
    const byte msgIndexTable[12] = { byte(11), byte(12), byte(13), byte(21), byte(22), byte(23), byte(31), byte(32), byte(33), byte(41), byte(42), byte(43) };
#endif

char showBuffer[31];
char httpHost[22]="Host: ";

int counter = 0;  
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
    Serial.println(F("\r\n---"));
  #endif

  #include "FRStackAPI.h"

  for (byte i = 0; i < ArraysSize; i++) { FRStackLastValue[i] = FRStackCmdDefaultValue[i]; }  // Init actual values from default

  // strcpy(httpHost, "Host: "); 
  strcat(httpHost, (String(FRStackPC[0]) + "." + String(FRStackPC[1]) + "." + String(FRStackPC[2]) + "." + String(FRStackPC[3])).c_str());
  #ifdef DEBUG
    Serial.print(httpHost);
    Serial.print(F("\t lenght:"));
    Serial.println(String(strlen(httpHost)));     
  #endif

  #ifdef DISPLAY_OLED091_I2C
    if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      #ifdef DEBUG
        Serial.println(F("SSD1306 allocation failed"));
      #endif
    for (;;)
    ;  // Don't proceed, loop forever
    }
    else
    {
    #ifdef DEBUG
      Serial.println(F("OLED091 Initiated"));
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
    }
    else
    {
    #ifdef DEBUG
      Serial.println(F("OLED130 Initiated"));
    #endif
    }
  #endif //DISPLAY_OLED130_I2C

  oled.setTextColor(1, 0);

  Ethernet.init(EthCSPin);

  oledShow("INIT ETH", 0, smallText, firstLine, showAttributeFalse);

  // start the Ethernet connection:
  #ifdef STATIC_IP
   Ethernet.begin(mac, myIP, myDns, myGateway, mySubnet);     // Configure using IP address defined in EthConfig.h 
  #else
    if (Ethernet.begin(mac) == 0) {
      #ifdef DEBUG
        Serial.println(F("Failed to configure Ethernet using DHCP"));
      #endif
    }
  #endif //STATIC-IP
  // Check for Ethernet Status 
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
  #ifdef DEBUG
    Serial.println(Ethernet.localIP());
  #endif

  IPAddress ip = Ethernet.localIP();

  strcpy(showBuffer, (String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3])).c_str());
  oledShow(showBuffer,0, smallText, firstLine, showAttributeFalse);

  delay(1000);  // give the Ethernet shield a second to initialize:

  oledShow("connecting FRStackPC", 0, smallText, firstLine, showAttributeFalse);
  delay(1000);  //give the user some time to look at the display


  if (client.connect(FRStackPC, FRStackPort)) {   // if you get a connection, report back via OLED:
    oledShow(httpHost, 0, smallText, firstLine, showAttributeFalse);
    delay(500);
    // Init some FRStack parameters to their default values
    oledShow("Set default values", 0, smallText, firstLine, showAttributeFalse);
    sendFRStackMsg(0, 0, FRStackCmdDefaultValue[0]);  //Volume
    delay(500);
    sendFRStackMsg(0, 3, FRStackCmdDefaultValue[3]);  //AGCT
    delay(500);
  }
  else {
    oledShow("Connection failed", 0, smallText, firstLine, showAttributeFalse);
    delay(5000);
  }
  strcpy (showBuffer, CALL_SIGN);
  strcat (showBuffer, " ");
  strcat (showBuffer, CODE_VERSION);
  strcat (showBuffer, " ");
  strcat (showBuffer, CODE_DATE);
  oledShow(showBuffer, 0, smallText, firstLine, showAttributeFalse);
  
  //Buttons handlers
  #ifdef DEBUG
    Serial.println(F("ButtonHandler1 created"));
  #endif
  enc1Btn.onPress(pressHandler1)
    .onDoublePress(pressHandler1)      // default timeout
    .onPressFor(pressHandler1, 1000);  // custom timeout for 1 second

  #if (ENCODERS > 1)
    #ifdef DEBUG
      Serial.println(F("ButtonHandler1 created"));
    #endif
    enc2Btn.onPress(pressHandler2)
      .onDoublePress(pressHandler2)      // default timeout
      .onPressFor(pressHandler2, 1000);  // custom timeout for 1 second
  #endif // ENCODERS > 1
  #if (ENCODERS > 2)
    #ifdef DEBUG
      Serial.println(F("ButtonHandler3 created"));
    #endif
    enc3Btn.onPress(pressHandler3)
      .onDoublePress(pressHandler3)      // default timeout
      .onPressFor(pressHandler3, 1000);  // custom timeout for 1 second
  #endif // ENCODERS > 2
  #if (ENCODERS > 3)
    #ifdef DEBUG
      Serial.println(F("ButtonHandler4 created"));
    #endif
    enc4Btn.onPress(pressHandler4)
      .onDoublePress(pressHandler4)      // default timeout
      .onPressFor(pressHandler4, 1000);  // custom timeout for 1 second
  #endif // ENCODERS > 3
}
// End setup()

void loop() {
  //Check for button press
  enc1Btn.read();

  // Check for encoders movement
  encChanged = false;
  static int oldcounter = 0;
    counter = enc1.read();
    if (counter != oldcounter) {
    //      if ((counter = enc1.readAndReset()) != 0) {
    Serial.print(counter);
    //if((counter = enc1.get_diffPosition()) != 0) {
    encCounter = counter / ENCODERS_STEPS;
    Serial.print("\t");
    Serial.println(encCounter);
    msgIndex = searchMesageIndex(1, enc1BtnStatus);
    oldcounter=counter;
    encChanged = true;
  }

  #if (ENCODERS > 1)
    enc2Btn.read();
    if ((counter = enc2.readAndReset()) != 0) {
      //  if((counter = enc2.get_diffPosition()) != 0) {
      encCounter = counter;
      msgIndex = searchMesageIndex(2, enc2BtnStatus);
      encChanged = true;
    }
  #endif

  #if (ENCODERS > 2)
    enc3Btn.read();
    if ((counter = enc3.readAndReset()) != 0) {
      //  if((counter = enc3.get_diffPosition()) != 0) {
      encCounter = counter / ENCODERS_STEPS;
      msgIndex = searchMesageIndex(3, enc3BtnStatus);
      encChanged = true;
  }
  #endif

  #if (ENCODERS > 3)
    enc4Btn.read();
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
// End loop()

// ------------------- FUNCTIONS ---------------------

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
  char clientMsg [50] = "GET ";
  if (messageType == 0)  //Standard message
    {
      strcat (clientMsg, FRStackCmdString[messageIndex]);
      strcat (clientMsg, String(functionValue).c_str());
    }
  if (messageType == 1) //Togle message
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
  #ifdef DEBUG
    Serial.println(clientMsg);
  #endif
}  //End sendFRStackMsg

//Display Functions on OLED
void oledShow(const char *messageText, int functionValue, byte textSize, displayLines lineNumber, showAttribute showAttribute) {
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
  #ifdef DEBUG
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
      oledShow(FRStackCmdLabel[msgIndex], -32768, 2, firstLine, showAttributeFalse);
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
} 
#endif  //End pressHandler3

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
}  
#endif //End pressHandler4
