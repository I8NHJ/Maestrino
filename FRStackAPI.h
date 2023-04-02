// Declare FRStack APIs, Default values, and ON/OFF strings

//Encoder 1
  FRStackCmdString[0] = "/api/ActiveSlice/AUDIOGAIN?param=+";
  FRStackCmdDefaultValue[0] = 30;
  FRStackCmdLowLimit[0] = 0;
  FRStackCmdHighLimit[0] = 100;
  FRStackCmdSteps[0] = 1;
  FRStackCmdLabel[0] = "Volume";
  FRStackCmdTogleString[0] = "/api/ActiveSlice/MUTE?param=";
  FRStackCmdTogleParameter[0] = 2;

  FRStackCmdString[1] = "/api/ActiveSlice/RITFREQ?param=";
  FRStackCmdDefaultValue[1] = 0;
  FRStackCmdTogleString[1] = "/api/ActiveSlice/RIT?param=";
  FRStackCmdLabel[1] = "RIT";
  FRStackCmdLowLimit[1] = 0;
  FRStackCmdHighLimit[1] = 1000;
  FRStackCmdSteps[1] = 1;
  FRStackCmdTogleParameter[1] = 2;
   
  FRStackCmdString[2] = "/api/ActiveSlice/XITFREQ?param=";
  FRStackCmdDefaultValue[2] = 0;
  FRStackCmdTogleString[2] = "/api/ActiveSlice/XIT?param=";
  FRStackCmdLabel[2] = "XIT";
  FRStackCmdLowLimit[2] = 0;
  FRStackCmdHighLimit[2] = 1000;
  FRStackCmdSteps[2] = 1;
  FRStackCmdTogleParameter[2] = 2;

//Encoder 2
  FRStackCmdString[3] = "/api/ActiveSlice/AGCLEVEL?param=+";
  FRStackCmdDefaultValue[3] = 50;
  FRStackCmdTogleString[3] = "/api/ActiveSlice/AGCMODE?param=DOWN";
  FRStackCmdLabel[3] = "AGCT";
  FRStackCmdLowLimit[3] = 0;
  FRStackCmdHighLimit[3] = 100;
  FRStackCmdSteps[3] = 1;
  FRStackCmdTogleParameter[3] = -32768;

  FRStackCmdString[4] = "/api/ActiveSlice/NBLEVEL?param=+";
  FRStackCmdDefaultValue[4] = 50;
  FRStackCmdTogleString[4] = "/api/ActiveSlice/NB?param=";
  FRStackCmdLabel[4] = "NB";
  FRStackCmdLowLimit[4] = 0;
  FRStackCmdHighLimit[4] = 100;
  FRStackCmdSteps[4] = 1;
  FRStackCmdTogleParameter[4] = 2;
  
  FRStackCmdString[5] = "/api/ActiveSlice/NRLEVEL?param=+";
  FRStackCmdDefaultValue[5] = 50;
  FRStackCmdTogleString[5] = "/api/ActiveSlice/NR?param=";
  FRStackCmdLabel[5] = "NR";
  FRStackCmdLowLimit[5] = 0;
  FRStackCmdHighLimit[5] = 100;
  FRStackCmdSteps[5] = 1;
  FRStackCmdTogleParameter[5] = 2;
  
  //Encoder 3
  FRStackCmdString[6] = "/api/ActiveSlice/FILTERLOW?param=";
  FRStackCmdDefaultValue[6] = 100;
  FRStackCmdTogleString[6] = "/api/ActiveSlice/FILTER?param=up";
  FRStackCmdLabel[6] = "FilLo";
  FRStackCmdLowLimit[6] = -6000;
  FRStackCmdHighLimit[6] = 6000;
  FRStackCmdSteps[6] = 25;
  FRStackCmdTogleParameter[6] = -32768;

  FRStackCmdString[7] = "/api/ActiveSlice/APFLEVEL?param=+";
  FRStackCmdDefaultValue[7] = 50;
  FRStackCmdTogleString[7] = "/api/ActiveSlice/APF?param=";
  FRStackCmdLabel[7] = "APF";
  FRStackCmdLowLimit[7] = 0;
  FRStackCmdHighLimit[7] = 100;
  FRStackCmdSteps[7] = 1;
  FRStackCmdTogleParameter[7] = 2;
  
  FRStackCmdString[8] = "/api/Radio/CWPITCH?param=";
  FRStackCmdDefaultValue[8] = 450;
  FRStackCmdTogleString[8] = "/api/ActiveSlice/CWSIDETONE?param=";
  FRStackCmdLabel[8] = "CWTone";
  FRStackCmdLowLimit[8] = 300;
  FRStackCmdHighLimit[8] = 1000;
  FRStackCmdSteps[8] = 1;
  FRStackCmdTogleParameter[87] = 2;

  
  //Encoder 4
  FRStackCmdString[9] = "/api/ActiveSlice/FILTERHIGH?param=";
  FRStackCmdDefaultValue[9] = 2800;
  FRStackCmdTogleString[9] = "/api/ActiveSlice/FILTER?param=down";
  FRStackCmdLabel[9] ="FilHi";
  FRStackCmdLowLimit[9] = -6000;
  FRStackCmdHighLimit[9] = 6000;
  FRStackCmdSteps[9] = 25;
  FRStackCmdTogleParameter[9] = -32768;
  
  // FRStackCmdString[10] = "/api/Radio/Slice?param=";
  // FRStackCmdDefaultValue[10] = 0;
  // FRStackCmdTogleString[10] = "/api/Radio/Slice?param=CREATE";
  // FRStackCmdLabel[10] = "A/B";
  // FRStackCmdLowLimit[10] = 0;
  // FRStackCmdHighLimit[10] = 7;
  // FRStackCmdSteps[10] = 1;
  // FRStackCmdTogleParameter[10] = -32768;

    
  FRStackCmdString[10] = "/api/ActiveSlice/RFGAIN?param=";
  FRStackCmdDefaultValue[10] = 0;
  FRStackCmdTogleString[10] = "/api/ActiveSlice/RFGAIN?param==down";
  FRStackCmdLabel[10] = "RFGAIN";
  FRStackCmdLowLimit[10] = -8;
  FRStackCmdHighLimit[10] = +32;
  FRStackCmdSteps[10] = 8;
  FRStackCmdTogleParameter[10] = -32768;

  FRStackCmdString[11] = "/api/Radio/CWDELAY?param=";
  FRStackCmdDefaultValue[11] = 250;
  FRStackCmdTogleString[11] = "/api/Radio/CWBREAKIN?param=";
  FRStackCmdLabel[11] = "CW Del";
  FRStackCmdLowLimit[11] = 0;
  FRStackCmdHighLimit[11] = 1000;
  FRStackCmdSteps[11] = 1;
  FRStackCmdTogleParameter[11] = 2;
 