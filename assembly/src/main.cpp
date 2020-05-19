#include <stdint.h>
#include <SPI.h>
#include <Arduino.h>
#include <uHIFA.h>

#define START_SENSOR _A14
#define BALL_SENSOR _A15
#define CONVEYOR_BELT _D23
#define PISTON_END _D4

//VAR DECLARATIONS START
uint8_t StopPins[4] = {_A9, _A10, _A11, _A12};

bool objectAtStart;
bool ballsLeft;
uint8_t conveyorBeltState;
uint8_t endPistonState = LOW;

Shuttle assembler(_D9, _D8);
Conveyor assemblyConveyor(_D0, _D1, _A0, _A1, _IN0);

bool reseting_station = true;
uint8_t index = 0x0;
uint8_t ballsPerObj = 2;
uint8_t ballsDelivered = 0;
uint8_t ballDelivIndex = 0x0;

//VAR DECLARATIONS END

//FUNC DECLARATIONS START

//FUNC DECLARATIONS END

void setup() {
    pinMode(START_SENSOR, INPUT);
    pinMode(BALL_SENSOR, INPUT);
    pinMode(CONVEYOR_BELT, OUTPUT);
    pinMode(PISTON_END, OUTPUT);
    assembler.config(VACUUM, _A6, _A5, _D3, _A4, _D5);
    assembler.init();
    for(int i = 0; i<4; i++){
        assembler.addStop(i, StopPins[i]);
    }
    assemblyConveyor.setMax(82);
    assemblyConveyor.init();
    Serial.begin(9600);
}

void loop() {
    //READ START
    objectAtStart = digitalRead(START_SENSOR);
    ballsLeft = digitalRead(BALL_SENSOR);
    assembler.scan();
    assemblyConveyor.scan();
    //READ END

    //LOGIC START
    if(reseting_station){
        assembler.move(0);
        assemblyConveyor.move(MIN);
        if(assembler.get(POSITION)==0&&assemblyConveyor.get(MIN)){
            index = 0x0;
            reseting_station = false;
        }
    }

    if(!reseting_station&&index==0x0&&objectAtStart){
        assembler.beginDeliv(EXTENDED);
        if(assembler.get(DELIVERING)){
            index = 0x1;
        }
    }
    
    if(!reseting_station&&index==0x1&&assembler.get(DELIVERING)){
        if(assembler.get(SAFE)){
            index = 0x2;
        }
    }

    if(!reseting_station&&index==0x2){
        assembler.move(3);
        if(assembler.get(POSITION)==3){
            index=0x3;
        }
        
    }

    if(!reseting_station&&index==0x3){
        assembler.endDeliv(EXTENDED);
        if(!assembler.get(DELIVERING)&&assembler.get(SAFE)){
            index = 0x4;
        }
    }

    if(!reseting_station&&index==0x4){
        assembler.move(1);
        if(assembler.get(POSITION)==1){
            index = 0x5;
        }
    }
    
    if(!reseting_station&&index==0x5&&ballsDelivered<ballsPerObj){
        if(ballDelivIndex==0x0){
            assembler.beginDeliv(EXTENDED);
            if(assembler.get(DELIVERING)&&assembler.get(SAFE)&&assembler.wait(500)){
                ballDelivIndex = 0x1;
            }
        }

        if(ballDelivIndex==0x1){
            assembler.move(3);
            if(assembler.get(POSITION)==3&&!assembler.get(MOVING)&&assembler.wait(500)){
                ballDelivIndex=0x2;
            }
        }

        if(ballDelivIndex==0x2){
            assembler.endDeliv(RETRACTED);
            if(!assembler.get(DELIVERING)&&assembler.get(SAFE)&&assembler.wait(500)){
                ballDelivIndex=0x3;
            }
        }

        if(ballDelivIndex==0x3&&assembler.get(SAFE)){
            assembler.move(1);
            if(assembler.get(POSITION)==1&&assembler.wait(500)){
                ballDelivIndex = 0x0;
                ballsDelivered += 1;
            }
        }
    }
    if(!reseting_station&&index==0x5&&ballsDelivered==ballsPerObj){
        ballsDelivered = 0;
        index = 0x6;
    }

    if(!reseting_station&&index==0x6){
        assembler.move(2);
        if(assembler.get(POSITION)==2){
            index = 0x7;
        }
    }

    if(!reseting_station&&index==0x7){
        assembler.beginDeliv(EXTENDED);
        if(assembler.get(DELIVERING)&&assembler.get(SAFE)){
            index = 0x8;
        }
    }

    if(!reseting_station&&index==0x8){
        assembler.move(3);
        if(assembler.get(POSITION)==3){
            index = 0x9;
        }
    }    

    if(!reseting_station&&index==0x9){
        assembler.endDeliv(RETRACTED);
        if(!assembler.get(DELIVERING)&&assembler.get(SAFE)){
            index=0xA;
        }
    }

    if(!reseting_station&&index==0xA){
        assembler.move(0);
        assemblyConveyor.move(MAX);
        if(assembler.get(POSITION)==0&&assemblyConveyor.get(MAX)){
            index = 0xB;
        }
    }

    if(!reseting_station&&index==0xB){
        endPistonState = HIGH;
        if(assemblyConveyor.wait(1000)){
            endPistonState = LOW;
            index = 0xC;
        }
    }

    if(!reseting_station&&index==0xC){
        assemblyConveyor.move(MIN);
        if(assemblyConveyor.get(MIN)){
            index = 0x0;
        }
    }

    if(not objectAtStart){
        conveyorBeltState = HIGH;
    }else{
        conveyorBeltState = LOW;
    }
    //LOGIC END

    //WRITE START
    assembler.update();
    assemblyConveyor.update();
    digitalWrite(CONVEYOR_BELT, conveyorBeltState);
    digitalWrite(PISTON_END, endPistonState);
    //WRITE END

    //DEBUG START
    Serial.print(assembler.get(SAFE));
    Serial.print("| index:");
    Serial.print(index);
    Serial.print("| ball index:");
    Serial.println(ballDelivIndex);
    //DEBUG END
}