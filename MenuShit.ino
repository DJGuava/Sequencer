
//All the menu text in flash, to save RAM
const PROGMEM char gate0[] = "none";
const PROGMEM char gate1[] = "once";
const PROGMEM char gate2[] = "hold";
const PROGMEM char gate3[] = "pulse";
const char *const gateModes[] PROGMEM = {gate0, gate1, gate2, gate3};

const PROGMEM char notxt[] = "";
const PROGMEM char txt0[] = "Sync";
const PROGMEM char txt1[] = "Digitals";
const PROGMEM char txt2[] = "Patterns";
const PROGMEM char txt3[] = "Analog 1";
const PROGMEM char txt4[] = "Analog 2";
const PROGMEM char txt5[] = "Output 1";
const PROGMEM char txt6[] = "Output 2";
const PROGMEM char txt7[] = "Output 3";
//const PROGMEM char txt8[] = "Output 4";
const PROGMEM char txt10[] = "Length:";
const PROGMEM char txt11[] = "Patterns";
const PROGMEM char txt13[] = "Repeats";
const PROGMEM char txt15[] = "Note:";
const PROGMEM char txt16[] = "Tone:";
//const PROGMEM char txt17[] = "Length:"; //Use txt10
const PROGMEM char txt18[] = "Gate mode:";
const PROGMEM char txt19[] = "Advanced";
const PROGMEM char txt20[] = "Slide:";
const PROGMEM char txt21[] = "Spice:";
const PROGMEM char txt25[] = "Page #:";
const char *const menu[] PROGMEM = {
                              txt0, txt1, txt2, txt3, txt4,
                              txt5, txt6, txt7, notxt, notxt,
                              txt10, txt11, notxt, txt13, notxt,
                              txt15, txt16, txt10, txt18, txt19, 
                              txt20, txt21, notxt, notxt, notxt,
                              txt25, notxt, notxt, notxt, notxt,
                              };


char buffer[10];

void paramPrint(){
  if(menuNav == 10){
    paramPrint(0, lengthDO[outSel]);
    ComposePrint();
  } else if(menuNav == 15){
    uint8_t tempTone = aSeq[outSel][subCur] /537;
    TonePrint(tempTone, aSeq[outSel][subCur] % 537);
    paramPrint(0, subCur);
    paramPrint(2, aLength[outSel][subCur]);
    GatePrint();
  } else if(menuNav == 20){
    paramPrint(0, aSlide[outSel][subCur]);
    paramPrint(1, aSpice[outSel][subCur]);
  } else if(menuNav == 25){
    paramPrint(0, page);
    display.fillRect(15, 12, 110, 50, BLACK);
    for(uint8_t i = 0; i < 4; i++){
      PatternPrint(i);
    }
  }
  display.display();
}

void ComposePrint(){
  display.fillRect(16, 24, 112, 12, BLACK);
  display.fillRect(16, 48, 112, 12, BLACK);
  for(uint8_t b = 0; b < lengthDO[outSel]; b++){
    display.setCursor(b * 14 + 16, 24);
    display.print(composition[outSel][b]);
    display.setCursor(b * 14 + 16, 48);
    display.print(repeats[outSel][b]);
  }
}

void ComposePrint(uint8_t pos, uint8_t val){
  uint8_t y = pos / 8;
  uint8_t x = pos % 8;
  display.fillRect(x * 14 + 16, y * 24 + 24, 14, 12, BLACK);
  display.setCursor(x * 14 + 16, y * 24 + 24);
  display.print(val);
}

void TonePrint(uint8_t note, uint16_t detune){
  display.fillRect(80, 12, 48, 12, BLACK);
  display.setCursor(80, 12);
  display.println(note);
  display.setCursor(92, 12);
  display.println('+');
  display.setCursor(98, 12);
  display.println(detune);
}

void PatternPrint(uint8_t line){
  display.setCursor(15, 12 * (line + 1));
  display.println(page * 4 + line + 1);
  for(uint8_t i = 0; i < 8; i++){
    if(patterns[page*4 + line] & (1 << i)){
      display.fillRect(30 + 10 * i, (line + 1) * 12, 6, 6, WHITE);
    } else {
      display.drawRect(30 + 10 * i, (line + 1) * 12, 6, 6, WHITE);
    }
  }
}

void PatternPrint(uint8_t line, uint8_t pos, bool val){
  if(val){
    display.fillRect(30 + 10 * pos, (line + 1) * 12, 6, 6, WHITE);
  } else {
    display.fillRect(31 + 10 * pos, 13 + line * 12, 4, 4, BLACK);
    display.drawRect(30 + 10 * pos, (line + 1) * 12, 6, 6, WHITE);
  }
}

void paramPrint(uint8_t line, uint8_t val){
  display.fillRect(80, line*12, 48, 12, BLACK);
  display.setCursor(80, line*12);
  display.println(val);
}

void GatePrint(){
  display.fillRect(80, 36, 48, 12, BLACK);
  if(aGate[outSel][subCur] > 3){
    return;
  }
  display.setCursor(80, 36);
//  uint8_t index =aGate[subCur];
  strcpy_P(buffer, (char *)pgm_read_word(&(gateModes[aGate[outSel][subCur]])));
  display.print(buffer);
}

void TextUpdate(uint8_t index){
  lastNav = index;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  for(uint8_t i = 0; i < 5; i++){
    display.setCursor(15, 12*i);
    strcpy_P(buffer, (char *)pgm_read_word(&(menu[index + i])));
    display.print(buffer);
  }
  menuSel = 0;
  Pointer(0);
}

uint8_t CompPointer(uint8_t pos, int8_t direct){
  uint8_t y = 34 + 24 * floor(pos / 8);
  uint8_t x = 14 * (pos % 8);
  display.drawLine(15 + x, y, 29 + x, y, BLACK);
  uint8_t tempPos = constrain(pos + direct, 0, 2 * Digital_Length - 1);
  y = 34 + 24 * floor(tempPos / 8);
  x = 14 * (tempPos % 8);
  display.drawLine(15 + x, y, 29 + x, y, WHITE);
  return tempPos;
}

uint8_t PatternPointer(uint8_t pos, int8_t direct){
  uint8_t y = 8 + 12 * menuSel;
  display.drawLine(29 + 10 * pos, y, 36 + 10 * pos, y, BLACK);
  uint8_t tempPos = constrain(pos + direct, 0, 7);
  display.drawLine(29 + 10 * tempPos, y, 36 + 10 * tempPos, y, WHITE);
  return tempPos;
}

uint8_t Pointer(uint8_t pos){
  display.fillRect(1, pos*12, 10, 7, BLACK);
  uint8_t newPos = constrain(pos + dir, 0, 4);
  uint8_t tempPos = 12*newPos;
  display.drawLine(1, tempPos, 10, tempPos + 3, WHITE);
  display.drawLine(1, tempPos + 6, 10, tempPos + 3, WHITE);
  display.display();
  dir = 0;
  return newPos;
}
