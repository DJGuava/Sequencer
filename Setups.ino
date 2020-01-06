inline void PinSetup(){
  //Pin directions
  pinMode(OUTG1, OUTPUT);
  pinMode(OUTG2, OUTPUT);
  pinMode(OUTD1, OUTPUT);
  pinMode(OUTD2, OUTPUT);
  pinMode(OUTD3, OUTPUT);
  pinMode(OUTD4, OUTPUT);
  pinMode(RELeft, INPUT);
  pinMode(RERight, INPUT);
  pinMode(Button1, INPUT);
  pinMode(Button2, INPUT);
  pinMode(ExtCLK, INPUT_PULLUP);
}

//Loads patterns and values stored in EEPROM
inline void LoadValues(){
  //Load patterns
  for(uint8_t i = 0; i < Pattern_Size; i++){
    EEPROM.get(EEPATTERN + i, patterns[i]);
  }
  //Load Digital shit
  for(uint8_t i = 0; i < Digital_Outs; i++){
    EEPROM.get(EEDLENGTH + i, lengthDO[i]);
    for(uint8_t a = 0; a < Digital_Length; a++){
      EEPROM.get(EEDCOMP + i * Digital_Length + a, composition[i][a]);
      EEPROM.get(EEDREP + i * Digital_Length + a, repeats[i][a]);
    }
  }
  //Load Analog shit
  for(uint8_t a = 0; a < 2; a++){
    for(uint8_t b = 0; b < Analog_Length; b++){
      EEPROM.get(EEASEQ +  2 * (a * Analog_Length + b), aSeq[a][b]);
      EEPROM.get(EEALENGTH + a * Analog_Length + b, aLength[a][b]);
      EEPROM.get(EEAGATE + a * Analog_Length + b, aGate[a][b]);
      EEPROM.get(EEASLIDE + a * Analog_Length + b, aSlide[a][b]);
      EEPROM.get(EEASPICE + a * Analog_Length + b, aSpice[a][b]);
    }
  }
}

void SaveValue(){
  switch(menuNav + menuSel){
    case 10: {
      //Digital Length
      EEPROM.put(EEDLENGTH + outSel, lengthDO[outSel]);
      break;
    }
    case 11:
    case 12:
    case 13:
    case 14: {
      //Digital Composition
      for(uint8_t i = 0; i < Digital_Length; i++){
        EEPROM.put(EEDCOMP + outSel * Digital_Length + i, composition[outSel][i]);
        EEPROM.put(EEDREP + outSel * Digital_Length + i, repeats[outSel][i]);
      }
      break;
    }
    case 16: {
      //Analog sequence
      EEPROM.put(EEASEQ + 2 * (outSel * Analog_Length + subCur), aSeq[outSel][subCur]);
      break;
    }
    case 17: {
      //Analog lengths
      EEPROM.put(EEALENGTH + outSel * Analog_Length + subCur, aLength[outSel][subCur]);
      break;
    }
    case 18: {
      //Analog gates
      EEPROM.put(EEAGATE + outSel * Analog_Length + subCur, aGate[outSel][subCur]);
      break;
    }
    case 20: {
      //Analog slide
      EEPROM.put(EEASLIDE + outSel * Analog_Length + subCur, aSlide[outSel][subCur]);
      break;
    }
    case 21: {
      //Analog spice
      EEPROM.put(EEASPICE + outSel * Analog_Length + subCur, aSpice[outSel][subCur]);
      break;
    }
    case 26:
    case 27:
    case 28:
    case 29: {
      //Patterns
      EEPROM.put(EEPATTERN + page * 4 + menuSel - 1, patterns[page * 4 + menuSel - 1]);
      break;
    }
  }
}

inline void DisplaySetup(){
  //Display setup, and initial menu screen
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.display();
  TextUpdate(0);
}

inline void InterruptSetup(){
  //Create all the interrupts
  attachPCINT(digitalPinToPCINT(RELeft), RotEnc, CHANGE);
  attachPCINT(digitalPinToPCINT(RERight), RotEnc, CHANGE);
  attachPCINT(digitalPinToPCINT(Button1), Confirm, FALLING);
  attachPCINT(digitalPinToPCINT(Button2), Back, FALLING);
  attachInterrupt(digitalPinToInterrupt(ExtCLK), Step, CHANGE);
}
