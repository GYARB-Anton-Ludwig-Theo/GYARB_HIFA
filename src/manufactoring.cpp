#include <stdint.h>
#include <SPI.h>
#include <Arduino.h>
#include <uHIFA.h>

uint8_t index = 0;
uint8_t Stop_pins[4] = {_A9, _A10, _A11, _A12};
SHUTTLE constructor(_D9, _D8);

void move_loop(){
  uint8_t index = 0;
  if(constructor.status(POSITION) != index){
    constructor.move(index);
  }else{
    index += 1;
    if(index<3){
      index = 1;
    }
  }
}

void setup() {
  Serial.begin(9600);
  constructor.initiate();
  constructor.config(VACUUM, _A6, _A5, _D3, _A4, _D5);
  for(uint8_t i = 0; i<4; i++){
    constructor.addStop(i, Stop_pins[i]);
  }
}

void loop() {
  constructor.maintain();
  constructor.move(0);
  if(constructor.status(POSITION) == 0){
      constructor.beginDeliv(EXTENDED);
  }
  if(constructor.status(DELIVERING)){
    constructor.move(3);
  }
  
  if(constructor.status(POSITION) == 3){
    if(constructor.status(DELIVERING)){
      constructor.endDeliv(EXTENDED);
    }
  }

  
  /*if(constructor.status(POSITION) == index){
    index += 1;
    delay(1000);
    if(index>3){
      index = 0;
    }
  }*/

  Serial.print(index);
  Serial.print("\t");
  Serial.println(constructor.status(POSITION));
}