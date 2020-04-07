#include <stdint.h>
#include <SPI.h>
#include <Arduino.h>
#include <uHIFA.h>
int8_t sequence_step=-1;
int8_t reset_sequence_step = 0;

bool finished_delivery = false;
bool clear_Conveyor_belt = false;
bool reset_req= true;
const uint8_t startPos = 0;
const uint8_t shelfPos[3] = {15, 52, 85};
const uint8_t endPos = 100;

uint8_t vertical_index = 0;
uint8_t horizontal_index = 0;
uint8_t next_horizontal_pos;

const int sensNum = 12+1;
int A_pin[sensNum]= {_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8, _A9, _A10, _A11, _A12};
int A_vals[sensNum]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const int actNum = 9+1;
int D_pin[actNum] = {_D0, _D1, _D2, _D3, _D4, _D5, _D6, _D7, _D8, _D9};
int D_status[actNum] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1};
uint8_t Stop_pins[4] = {_A9, _A10, _A11, _A12};

Shuttle storage_vertical(_D9, _D8);
Conveyor storage_horizontal(_D0, _D1, _A1, _A2, _A3);
void setup() {
  for(int i = 0; i<sensNum; i++){
    pinMode(A_pin[i], INPUT);
  }
  for(int i = 0; i<actNum; i++){
    pinMode(D_pin[i], OUTPUT);
  }
  storage_vertical.init();
  storage_vertical.config(CLAW, _A5, _A6, _D4, _A4, _D5);
  for(uint8_t i = 0; i<3; i++){
    storage_vertical.addStop(i, Stop_pins[i]);
  }
  storage_horizontal.init();
  storage_horizontal.setMax(27);
  Serial.begin(9600);
}

void loop() {
  storage_horizontal.scan();
  storage_vertical.scan();

  if(reset_req and sequence_step==-1){
    storage_horizontal.reset();
    storage_vertical.move(0);
    reset_sequence_step = 1;
  }
  if(storage_horizontal.get(SAFE) and storage_horizontal.get(MIN) and storage_vertical.get(POSITION)==0 and reset_sequence_step == 1){
      reset_req = false;
      clear_Conveyor_belt = false;
      next_horizontal_pos = shelfPos[horizontal_index];
      reset_sequence_step = 0;
      sequence_step += 1;
  }
  if(sequence_step==0 and storage_horizontal.get(SAFE) and not storage_horizontal.get(RESETING)){
    storage_horizontal.move(next_horizontal_pos);
    sequence_step += 1;
  }
  if(sequence_step==1 and storage_horizontal.get(POSITION) == next_horizontal_pos and not reset_req and not finished_delivery){
    storage_vertical.move(vertical_index);
    sequence_step += 1;
  }

  if(sequence_step==2 and storage_vertical.get(POSITION)==vertical_index){
      storage_vertical.beginDeliv(EXTENDED);
      sequence_step+=1;
  }

  if(sequence_step==3 and storage_vertical.get(DELIVERING) and not reset_req){
    storage_horizontal.move(MAX);
    sequence_step += 1;
  }

  if(sequence_step==4 and storage_horizontal.get(POSITION)==next_horizontal_pos and storage_vertical.get(DELIVERING) and not reset_req){
    storage_vertical.move(3);
    sequence_step += 1;
  }
  if(sequence_step==5 and storage_vertical.get(POSITION)==3 and storage_vertical.get(DELIVERING) and not reset_req){
    storage_vertical.endDeliv(RETRACTED);
    sequence_step += 1;
  }
  if(sequence_step==6 and storage_vertical.get(POSITION)==3 and not storage_vertical.get(DELIVERING) and not reset_req){
    finished_delivery = true;
    storage_vertical.move(0);
    sequence_step += 1;
  }
  if(sequence_step==7 and storage_vertical.get(POSITION)==0 and not reset_req and finished_delivery){
    horizontal_index += 1;
    if(horizontal_index > 2){
      horizontal_index = 0;
      vertical_index += 1;
    }
    if(vertical_index > 2){
      vertical_index = 0;
    }
    reset_req = true;
    finished_delivery =false;
    clear_Conveyor_belt = true;
    sequence_step = -1;
  }

  if(storage_vertical.get(POSITION) < 3){
    if(not storage_horizontal.get(RESET_REQ)){
      digitalWrite(_D2, 1);
    }
  }else{
    digitalWrite(_D2, 0);
  }
  
  storage_vertical.update();
  storage_horizontal.update();
  Serial.print(sequence_step);
  Serial.print("| Safe: ");
  Serial.print(storage_horizontal.get(SAFE));
  Serial.print("| fwrds: ");
  Serial.print(storage_horizontal.get(FORWARDS));
  Serial.print("| bwrds: ");
  Serial.print(storage_horizontal.get(BACKWARDS));
  Serial.print("| vert_pos: ");
  Serial.print(storage_vertical.get(POSITION));
  Serial.print("| hori_pos: ");
  Serial.println(storage_horizontal.get(POSITION));
}