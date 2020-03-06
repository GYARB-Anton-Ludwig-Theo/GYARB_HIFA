#include <stdint.h>
#include <SPI.h>
#include <Arduino.h>
#include <uHIFA.h>

bool reset_req = true;
CONVEYOR materialCheck(_D0, _D1, _A1, _A2, _A3);
PISTON C1;
 
void setup() {
    Serial.begin(9600);
    materialCheck.initiate();
    materialCheck.setMax(82);
    C1.initiate();
    C1.config(PUSH, _A13, _A14, _D3);
}
 
void loop() {
 
 
    materialCheck.maintain();
    if(reset_req){
        materialCheck.reset();
    }else{
        materialCheck.move(100);
    }

    if(not materialCheck.status(RESET_REQ)){
        reset_req = false;
    }else{
        reset_req = true;
    }
 
    Serial.print(C1.status(EXTENDED));
    Serial.print("\t");
    Serial.println(C1.status(RETRACTED));
 }
