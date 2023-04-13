const byte ENCODERS_STEPS = 4; //How many edges for 1 pulse

#ifdef MEGA
  // ENCODERS pins definition - MEGA
  const byte ENCODER_1_S1_PIN = 2;  // Interrupt
  const byte ENCODER_1_S2_PIN = 4;  // No Interrupt
  const byte ENCODER_2_S1_PIN = 3;  // Interrupt
  const byte ENCODER_2_S2_PIN = 5;  // No Interrupt
  const byte ENCODER_3_S1_PIN = 19; // Interrupt 18
  const byte ENCODER_3_S2_PIN = 17; // No Interrupt 16
  const byte ENCODER_4_S1_PIN = 18; // Interrupt
  const byte ENCODER_4_S2_PIN = 16; // No Interrupt

  // ENCODER Buttons pin definition - MEGA
  const byte enc1BtnPin = 6;   // Push button on encoder 1
  const byte enc2BtnPin = 7;   // Push button on encoder 2
  const byte enc3BtnPin = 15;  // Push button on encoder 3
  const byte enc4BtnPin = 14;  // Push button on encoder 4

  // Ethernet.init(pin) to configure the CS pin. Most Arduino boards use 10
  const byte EthCSPin = 10;   
#endif

#ifdef DUE
  // ENCODERS pins definition - DUE
  const int ENCODER_1_S1_PIN = 2;  // Interrupt
  const int ENCODER_1_S2_PIN = 4;  // No Interrupt
  const int ENCODER_2_S1_PIN = 18; // Interrupt
  const int ENCODER_2_S2_PIN = 17; // No Interrupt
  const int ENCODER_3_S1_PIN = 3;  // Interrupt
  const int ENCODER_3_S2_PIN = 6;  // No Interrupt
  const int ENCODER_4_S1_PIN = 19; // Interrupt
  const int ENCODER_4_S2_PIN = 14; // No Interrupt

  // ENCODER Buttons pin definition - DUE
  const int enc1BtnPin = 5;   // Push button on encoder 1
  const int enc2BtnPin = 16;  // Push button on encoder 2
  const int enc3BtnPin = 15;  // Push button on encoder 3
  const int enc4BtnPin = 7;   // Push button on encoder 4

  // Ethernet.init(pin) to configure the CS pin. Most Arduino boards use 10
  const byte EthCSPin = 10;  
#endif

#ifdef LEONARDO_ETH
  // ENCODERS pins definition - LEONARDO_ETH
  const byte ENCODER_1_S1_PIN = 2;  // Interrupt
  const byte ENCODER_1_S2_PIN = 4;  // No Interrupt
  const byte ENCODER_2_S1_PIN = 3;  // Interrupt
  const byte ENCODER_2_S2_PIN = 5;  // No Interrupt
  const byte ENCODER_3_S1_PIN = 19; // Interrupt 18
  const byte ENCODER_3_S2_PIN = 17; // No Interrupt 16
  const byte ENCODER_4_S1_PIN = 18; // Interrupt
  const byte ENCODER_4_S2_PIN = 16; // No Interrupt

  // ENCODER Buttons pin definition - LEONARDO_ETH
  const byte enc1BtnPin = 6;   // Push button on encoder 1
  const byte enc2BtnPin = 7;   // Push button on encoder 2
  const byte enc3BtnPin = 15;  // Push button on encoder 3
  const byte enc4BtnPin = 14;  // Push button on encoder 4

  // Ethernet.init(pin) to configure the CS pin. Most Arduino boards use 10
  const byte EthCSPin = 10;   
#endif
