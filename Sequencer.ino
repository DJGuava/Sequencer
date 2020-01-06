#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <PinChangeInterrupt.h>
#include <EEPROM.h>


//Parts of the digital menu is hardcoded NB!
#define Digital_Length 8
#define Analog_Length 32
#define Pattern_Size 32
#define Digital_Outs 3

#define ScreenWidth 128
#define ScreenHeight 64
#define OLED_Reset -1
Adafruit_SSD1306 display(ScreenWidth, ScreenHeight, &Wire, OLED_Reset);

#define OUTA1 9
#define OUTA2 10
#define OUTG1 11
#define OUTG2 12

#define OUTD1 A2
#define OUTD2 A1
#define OUTD3 A0
#define OUTD4 A3

#define ExtCLK 2

#define RELeft 3
#define RERight 4
#define Button1 6
#define Button2 5

/*
 * #define MidiRX 0
 * #define MidiTX 1
 */
//EEPROM positions
 #define EEDLENGTH 0
 #define EEDCOMP (EEDLENGTH + 4)
 #define EEDREP (EEDCOMP + 4 * Digital_Length)
 #define EEPATTERN (EEDREP + 4 * Digital_Length)
 #define EEASEQ (EEPATTERN + Pattern_Size)
 #define EEALENGTH (EEASEQ + 4 * Analog_Length)
 #define EEAGATE (EEALENGTH + 2 * Analog_Length)
 #define EEASLIDE (EEAGATE + 2 * Analog_Length)
 #define EEASPICE (EEASLIDE + 2 * Analog_Length)

int8_t lengthDO[Digital_Outs] = {2, 2, 2};           //Number of 8-step parts to the sequence
int8_t seqPos[Digital_Outs] = {0, 0, 0};             //What part of the sequence we at?
uint8_t composition[Digital_Outs][Digital_Length];   //Sequence of patterns
int8_t repeats[Digital_Outs][Digital_Length];       //Repeats of sequence step
uint8_t counts[Digital_Outs] = {0,0,0};
bool selected = false;

uint8_t patterns[Pattern_Size];     //Patterns used in composition
uint8_t page = 0;                   //Page index for patterns
uint8_t subCur = 0;                 //Cursor position when editing patterns and compositions

uint8_t seqA;             //Input A from rotary encoder
uint8_t seqB;             //Input B from rotary encoder
uint8_t lastStep;         //Used to reduce display calls
int8_t dir = 0;           //Direction the rotary encoder was turned
bool editing = false;     //Disables navigation to edit parameters
bool paramChange = false; //Has a parameter changed?
uint8_t outSel;           //Output selection
uint8_t currentStep = 0;  //yes
int lastMillis;           //Last value of millis
bool seeded = 0;          //Has seed been generated for Random?

int16_t aSeq[2][Analog_Length];         //Analog sequence
uint8_t aLength [2][Analog_Length];     //Lengths of analog notes
uint8_t aPos[2] = {0,0};                //Current position in sequence
uint8_t aCount [2] = {0,0};             //How many steps has current note been played
uint8_t aGate[2][Analog_Length];        //Gate-mode for each step
uint8_t aSlide[2][Analog_Length];       //Amount of slide a note has
uint16_t slideGoal[2];                  //Value to reach
uint8_t aSpice[2][Analog_Length];       //Randomness of each note

uint8_t menuSel = 0;      //Which line in current menu are we pointing at?
uint8_t menuNav = 0;      //Where in the menu array are we?
uint8_t lastNav;          //Previous menuNav, triggers display update

void setup() {
  SetupPWM16();
  PinSetup();
  DisplaySetup();
  LoadValues();
  InterruptSetup();
}

void loop() {
  int delta = millis() - lastMillis;
  lastMillis = millis();
  //Handle sliding
  for(uint8_t i = 0; i < 2; i++){
    if(slideGoal[i] != analogRead16(OUTA1 + i)){
      if(slideGoal[i] < analogRead16(OUTA1 + i)){
        analogWrite16(OUTA1 + i, max(slideGoal[i], analogRead16(OUTA1 + i) - delta/aSlide[i][aPos[i]]));
      } else {
        analogWrite16(OUTA1 + i, min(slideGoal[i], analogRead16(OUTA1 + i) + delta/aSlide[i][aPos[i]]));
      }
    }
  }
  
  if(editing){
    uint8_t superNav = menuNav + menuSel;   //Decide which parameter to change
    //If rotary encoder has been turned
    if(dir != 0){
      switch(superNav){
        //Digital sequence length
        case 10: {
          lengthDO[outSel] = constrain(lengthDO[outSel] + dir, 1, Digital_Length);
          paramPrint();
          break;
        }
        case 11:
        case 12:
        case 13:
        case 14: {
          if(selected){
            //Edit value
            if(subCur < 8){
              composition[outSel][subCur] = constrain(composition[outSel][subCur] + dir, 0, Pattern_Size);
              ComposePrint(subCur, composition[outSel][subCur]);
            } else {
              repeats[outSel][subCur - 8] = constrain(repeats[outSel][subCur - 8] + dir, 1, 99);
              ComposePrint(subCur, repeats[outSel][subCur - 8]);
            }
            //Update screen value
            
          } else {
            //Navigate menu
            subCur = CompPointer(subCur, dir);
          }
          break;
        }
        case 15: {
          subCur = constrain(subCur + dir, 0, Analog_Length - 1);
          paramPrint();
          break;
        }
        case 16: {
          //Rough tone, then detune
          if(selected){
            aSeq[outSel][subCur] = constrain(aSeq[outSel][subCur] + dir, 0, 0x7fff);
          } else {
            aSeq[outSel][subCur] = constrain(aSeq[outSel][subCur] + (dir * 537), 0, 0x7fff);
          }
          uint8_t tempTone = aSeq[outSel][subCur] / 537;
          TonePrint(tempTone, aSeq[outSel][subCur] % 537);
          break;
        }
        case 17: {
          aLength[outSel][subCur] += dir;
          paramPrint(2, aLength[outSel][subCur]);
          break;
        }
        case 18: {
          aGate[outSel][subCur] = constrain(aGate[outSel][subCur] + dir, 0, 3);
          GatePrint();
          break;
        }
        case 20: {
          aSlide[outSel][subCur] += dir;
          paramPrint(0, aSlide[outSel][subCur]);
          break;
        }
        case 21: {
          aSpice[outSel][subCur] += dir;
          paramPrint(1, aSpice[outSel][subCur]);
          break;
        }
        case 25: {
          page = constrain(page + dir, 0, (Pattern_Size / 4) - 1);
          paramPrint();
          break;
        }
        case 26:
        case 27:
        case 28:
        case 29: {
          subCur = PatternPointer(subCur, dir);
          
          break;
        }
      }
      dir = 0;
      display.display();
    }
    if(paramChange){
      paramChange = false;
      //Updates single square when editing patterns
      if(superNav > 25 && superNav < 30){
        PatternPrint(menuSel - 1, subCur, patterns[page*4 + menuSel - 1] & (1 << subCur));
      }
      display.display();
    }
  } else {
    if(dir != 0){
      menuSel = Pointer(menuSel);
    }
    if(menuNav != lastNav){
      TextUpdate(menuNav);
      paramChange = true;
    }
    if(paramChange){
      paramPrint();
      paramChange = false;
    }
  }
}

void RotEnc(){
  //Decide direction of rotation
  delay(3);
  seqA = (seqA << 1) | digitalRead(RELeft);
  seqB = (seqB << 1) | digitalRead(RERight);
  seqA &= 0x03;
  seqB &= 0x03;
  if((seqA == 0b00 && seqB == 0b10)||(seqA == 0b11 && seqB == 0b01)){
    dir = -1;
  } else if((seqA == 0b00 && seqB == 0b01)||(seqA == 0b11 && seqB == 0b10)){
    dir = 1;
  }
}

void Confirm() {
  switch(menuSel + menuNav){
    case 0: {
      currentStep = 0; 
      for(uint8_t i = 0; i < Digital_Length; i++){
        seqPos[i] = 0; 
        counts[i] = 0;
      }
      for(uint8_t b = 0; b < 2; b++){
        aPos[b] = 0;
        aCount[b] = 0;
      }
      break;
    }
    case 1: menuNav = 5; break;
    case 2: menuNav = 25; break;
    case 3: {
      outSel = 0;
      menuNav = 15;
      subCur = 0;
      break;
    }
    case 4: {
      outSel = 1;
      menuNav = 15;
      subCur = 0;
      break;
    }
    case 5:
    case 6:
    case 7: {
//    case 8: {
      outSel = menuSel;
      menuNav = 10;
      break;
    };
    case 10: editing = true; break;
    case 11:
    case 12:
    case 13:
    case 14: {
      if(selected){
        selected = false;
      } else if(editing){
        selected = true;
      } else {
        subCur = 8 * (menuSel/2);
        editing = true;
      }
      break;
    }
    case 15: editing = true; break;
    case 16: {
      if(editing){
        if(selected){
          selected = false;
          editing = false;
          SaveValue();
        } else {
          selected = true;
        }
      } else {
        editing = true;
      }
      break;
    }
    case 17:
    case 18: editing = true; break;
    case 19: {
      menuNav = 20;
      break;
    }
    case 20:
    case 21:
    case 25: editing = true; break;
    case 26:
    case 27:
    case 28:
    case 29: {
      if(editing){
        InvertBit(&patterns[page*4 + menuSel - 1], subCur);
        paramChange = true;
      } else {
        editing = true; 
        subCur = 0;
      }
      break;
    }
  }
}

void InvertBit(uint8_t* val, uint8_t leBit){
  if(*val & (1 << leBit)){
    *val &= ~(1 << leBit);
  } else {
    *val |= 1 << leBit;
  }
}

void Back() {
  if(selected){
    selected = false;
    paramChange = true;
  } else if(editing){
    paramChange = true;
    editing = false;
    SaveValue();
  } else {
    switch(menuNav){
      case 5: menuNav = 0; break;
      case 10: menuNav = 5; break;
      case 15: menuNav = 0; break;
      case 20: menuNav = 15; break;
      case 25: menuNav = 0; break;
    }
  }
  if(!seeded){
    randomSeed(millis());
    seeded = true;
  }
}

void Step() {
  
  //Set outputs high on rising edge
  delay(3);
  if(digitalRead(ExtCLK)){
    for(uint8_t a = 0; a < Digital_Outs; a++){
      if(composition[a][seqPos[a]] > 0){
        if(patterns[composition[a][seqPos[a]]-1] & (1 << currentStep)){
          SetOUTD(a, HIGH);
        }
      }
    }
    for(uint8_t b = 0; b < 2; b++){
      if(aGate[b][aPos[b]] == 0){
        continue;
      } else if(aGate[b][aPos[b]] == 1 && aCount[b] > 0){
        continue;
      }
      SetOUTG(b, HIGH);
      
    }
  //Set outputs low on falling edge
  } else {
    for(uint8_t a = 0; a < Digital_Outs; a++){
      SetOUTD(a, LOW);
    }
    
    currentStep++;
    if(currentStep > 7){
      currentStep = 0;
      for(uint8_t i = 0; i < Digital_Outs; i++){
        counts[i]++;
        if(counts[i] >= repeats[i]){
          counts[i] = 0;
          seqPos[i]++;
          if(seqPos[i] >= lengthDO[i]){
            seqPos[i] = 0;
          }
        }
      }
    }
    
    for(uint8_t b = 0; b < 2; b++){
      aCount[b]++;
      if(aCount[b] >= aLength[b][aPos[b]]){
        if(aPos[b] >= Analog_Length){
          aPos[b] = 0;
        } else if(aSeq[b][aPos[b] + 1] > 0){
          aPos[b]++;
        } else {
          aPos[b] = 0;
        }
        aCount[b] = 0;
        SetOUTG(b, LOW);
        uint16_t tempF = aSeq[b][aPos[b]] + aSpice[b][aPos[b]] * random(100);
        if(aSlide[b][aPos[b]] > 0){
          slideGoal[b] = tempF;
        } else {
          slideGoal[b] = tempF;
          analogWrite16(OUTA1 + b, tempF);
        }
      } else {
        if(!(aGate[b][aPos[b]] == 2)){
          SetOUTG(b, LOW);
        }
      }
    }
  }
}

void SetOUTD (int8_t output, bool state){
  switch(output){
    case 0: {
      digitalWrite(OUTD1, state);
      break;
    }
    case 1: {
      digitalWrite(OUTD2, state);
      break;
    }
    case 2: {
      digitalWrite(OUTD3, state);
      break;
    }
    case 3: {
      digitalWrite(OUTD4, state);
      break;
    }
    
  }
}

void SetOUTG(int8_t output, bool state){
  switch(output){
    case 0: {
      digitalWrite(OUTG1, state);
      break;
    }
    case 1: {
      digitalWrite(OUTG2, state);
      break;
    }
  }
}
