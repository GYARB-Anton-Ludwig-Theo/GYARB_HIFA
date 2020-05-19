#include <stdint.h>
#include <SPI.h>
#include <Arduino.h>
#include <uHIFA.h>

#define CONVEYOR_BELT _D23

int8_t index= 0x0;
bool reseting_station = true;

const uint8_t shelfPos[3] = {45, 156, 255};
uint8_t vertical_index = 0;
uint8_t horizontal_index = 0;
uint8_t next_horizontal_pos;

uint8_t Stop_pins[4] = {_A9, _A10, _A11, _A12};

Shuttle storageArm(_D9, _D8);
Conveyor storageConveyor(_D0, _D1, _A1, _A2, _IN0);
void setup() {
  pinMode(CONVEYOR_BELT, OUTPUT);
  storageArm.config(CLAW, _A5, _A6, _D4, _A4, _D5);
  storageArm.init();
  for(uint8_t i = 0; i<3; i++){
    storageArm.addStop(i, Stop_pins[i]);
  }
  storageConveyor.init();
  storageConveyor.setMax(310);
  Serial.begin(9600);
}

void loop() {
  storageArm.scan();
  storageConveyor.scan();

  if(reseting_station){
    storageArm.move(0);
    if(storageConveyor.get(DIRECTION_DEFAULT)&&storageConveyor.get(POSITION)!=0){
      storageConveyor.move(0);
    }else if(!storageConveyor.get(DIRECTION_DEFAULT)&&storageConveyor.get(POSITION)!=310){
      storageConveyor.move(310);
    }
    if(storageArm.get(POSITION)==0&&storageConveyor.get(POSITION)==(310||0)){
      reseting_station = false;
      next_horizontal_pos = shelfPos[horizontal_index];
      index = 0x0;
    }
  }

  if(!reseting_station&&index==0x0){
    storageConveyor.move(next_horizontal_pos);
    if(storageConveyor.get(POSITION) == next_horizontal_pos){
      index = 0x1;
    }
  }

  if(!reseting_station&&index==0x1){
    storageArm.move(vertical_index);
    if(storageArm.get(POSITION)==vertical_index){
      index = 0x2;
    }
  }

  if(!reseting_station&&index==0x2){
      storageArm.beginDeliv(EXTENDED);
      if(storageArm.get(DELIVERING)){
        index = 0x3;
      }
  }

  if(!reseting_station&&index==0x3){
    storageConveyor.move(MAX);
    if(storageConveyor.get(MAX)){
      index = 0x4;
    }
  }

  if(!reseting_station&&index==0x4){
    storageArm.move(3);
    if(storageArm.get(POSITION)==3){
      index = 0x5;
    }
  }

  if(!reseting_station&&index==0x5){
    storageArm.endDeliv(RETRACTED);
    if(!storageArm.get(DELIVERING)){
      index = 0x6;
    }
  }

  if(!reseting_station&&index==0x6){
    storageArm.move(0);
    if(storageArm.get(POSITION)==0){
      index = 0x7;
    }
  }

  if(!reseting_station&&index==0x7){
    horizontal_index++;
    if(horizontal_index > 2){
      horizontal_index = 0;
      vertical_index++;
    }
    if(vertical_index > 2){
      vertical_index = 0;
    }
    reseting_station = true;
  }

  if(!reseting_station and storageArm.get(POSITION)!=3){
    digitalWrite(CONVEYOR_BELT, HIGH);
  }else{
    digitalWrite(CONVEYOR_BELT, LOW);
  }
  
  storageArm.update();
  storageConveyor.update();
  Serial.print(storageConveyor.get(POSITION));
  Serial.print(" ");
  Serial.println(reseting_station);
}