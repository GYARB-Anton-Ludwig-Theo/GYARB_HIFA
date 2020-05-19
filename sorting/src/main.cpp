/* 
by Anton Härfstrand, Ludwig Lenz and Theo Johansson
*/
#include <stdint.h>
#include <Arduino.h>
#include <uHIFA.h>

#define METAL 0x0
#define WHITE 0x1
#define BLACK 0x2

uint8_t index = 0x0;
bool strState;     // start state sensor

bool material[3];
bool acceptedMaterial[3] = {true, true, false}; //{metal, white, black}
bool acceptedObject;

bool metalState;
bool infraredState;
bool objectState;

bool metalRead;
bool infraredRead;
bool objectRead;

uint8_t discard_position = 55;

bool reached_end = false;
bool reseting_station = true;
Conveyor sortingConveyor(_D0, _D1, _A1, _A2, _IN0); 
Piston C1;
Piston C2; 
 
void setup() {
    Serial.begin(9600);
    sortingConveyor.init();
    sortingConveyor.setMax(82);
    C1.config(_A13, _A14, _D3); //cylinder 1
    C2.config(_I16, _A15, _D2); //cylinder 2
    C1.init();
    C2.init();
    pinMode(_A4, INPUT);
    pinMode(_A5, INPUT);
    pinMode(_A6, INPUT);
    pinMode(_A9, INPUT);
}
 
void loop() {

    metalState = digitalRead(_A4);
    infraredState = digitalRead(_A5);
    objectState = digitalRead(_A6);
    strState = digitalRead(_A9);
    sortingConveyor.scan();
    C1.scan();
    C2.scan();


    if(metalState or infraredState or objectState){
        if(metalState){
            metalRead = true;
        }
        if(infraredState){
            infraredRead = true;
        }
        if(objectState){
            objectRead = true;
        }
        material[METAL] = (metalRead&&infraredRead&&objectRead) ? true:false;
        material[1] = (!metalRead&&infraredRead&&objectRead) ? true:false;
        material[2] = (!metalRead&&!infraredRead&&objectRead) ? true:false;
        if(material[METAL]){
            acceptedObject = (material[METAL]!=acceptedMaterial[METAL]) ? false:true;
        }
        if(material[WHITE]){
            acceptedObject = (material[WHITE]!=acceptedMaterial[WHITE]) ? false:true;
        }
        if(material[BLACK]){
            acceptedObject = (material[BLACK]!=acceptedMaterial[BLACK]) ? false:true;
        }
    }

    
    
    if(reseting_station){
        C1.retract();
        C2.retract();
        sortingConveyor.move(MIN);
        if(C1.get(RETRACTED)&&C2.get(RETRACTED)&&sortingConveyor.get(MIN)){
            reseting_station = false;
            index = 0x0;
            metalRead = false;
            infraredRead = false;
            objectRead = false;
            acceptedObject = false;
        }
    }
    


    if(!reseting_station&&index == 0x0) {
        if(strState){
            index = 0x1;
        }
        
    }

    if(!reseting_station&&index==0x1) {
        C1.push();
        if(C1.get(RETRACTED)&&!strState){
            index = 0x2;
        }
        
    }    
    
    if(!reseting_station&&index==0x2){ 
        C1.retract();
        sortingConveyor.move(discard_position);
        if(sortingConveyor.get(POSITION)>=discard_position&&sortingConveyor.wait(10000)){
            index = 0x3;
        }
    }
    
    if(!reseting_station&&index ==0x3){
        sortingConveyor.move(discard_position);
        if(acceptedObject){
            index = 0x5;
        }else{
            index = 0x4;
        }
        
    }

    if(!reseting_station&&index == 0x4){
        sortingConveyor.move(discard_position);
        C2.push();
        if(C2.get(EXTENDED)){
            reseting_station = true;
        }
    }

    if(!reseting_station&&index == 0x5){
        sortingConveyor.move(MAX);
        if(sortingConveyor.get(MAX)){
            index = 0x6;
        }
    }

    if(!reseting_station&&index == 0x6){
        C2.extend();
        if(C2.get(EXTENDED)){
            index = 0x7;
        }
    }

    if(!reseting_station&&index == 0x7){
        C2.retract();
        if(C2.get(RETRACTED)){
            reseting_station = true;
        }
    }

    C1.update();
    C2.update();
    sortingConveyor.update();
    Serial.print(sortingConveyor.get(SAFE));
    Serial.print(" ");
    Serial.print(index);
    Serial.print(" ");
    Serial.println(sortingConveyor.get(POSITION));
}