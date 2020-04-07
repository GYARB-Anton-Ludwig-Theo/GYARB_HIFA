/* 
by Anton Härfstrand, Ludwig Lenz and Theo Johansson
*/
#include <stdint.h>
#include <SPI.h>
#include <Arduino.h>
#include <uHIFA.h>


uint8_t index = 0;
bool strState;     // start state sensor

bool material[3];
bool acceptedMaterial[3] = {true, true, true}; //{metal, white, black}
bool acceptedObject;

bool metalState;
bool infraredState;
bool objectState;

bool metalRead;
bool infraredRead;
bool objectRead;

bool reached_end = false;
bool reset_req;
Conveyor sortingConveyor(_D0, _D1, _A1, _A2, _A3); 
Piston C1;
Piston C2; 
 
void setup() {
    Serial.begin(9600);
    sortingConveyor.init();
    sortingConveyor.setMax(82);
    C1.init();
    C1.config(_A13, _A14, _D3); //cylinder 1
    C2.config(_I16, _A15, _D2); //cylinder 2
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
C2.scan();
C1.scan();

if(metalState or infraredState or objectState){
    if(metalState){
        metalRead = true;
    }
    if(infraredState){
        infraredRead = true;
    }
    if(objectRead){
        objectRead = true;
    }
    material[0] = (metalRead&&infraredRead&&objectRead) ? true:false;
    material[1] = (!metalRead&&infraredRead&&objectRead) ? true:false;
    material[2] = (!metalRead&&!infraredRead&&objectRead) ? true:false;
}

for(int i; i<3; i++){
    if(material[i]){
        if(material[i]!=acceptedMaterial[i]){
            acceptedObject = false;
        }else{
            acceptedObject = true;
        }
    }
}

if(index == 0 && sortingConveyor.get(MIN) ) {
    index += 1;
}

if(index == 1 && strState  && sortingConveyor.get(MIN) ) {
    C1.push();
    index += 2;
}    
    
if(index == 2 && )


    C1.update();
    C2.update();
    sortingConveyor.update();
    Serial.print(sortingConveyor.get(RESET_REQ));
    Serial.print("\t");
    Serial.println(sortingConveyor.get(MAX));
 }

/*
    if(reset_req){
        sortingConveyor.reset();
        index == 0;
    }else{
        if(not reached_end){
            sortingConveyor.move(MAX);
            if(sortingConveyor.get(MAX)){
                reached_end = true;
            }
        }else{
            sortingConveyor.move(MIN);
            if(sortingConveyor.get(MIN)){
                reached_end =false;
            }
        }
        
    }

    if(not sortingConveyor.status(RESET_REQ)){
        reset_req = false;
    }else{
        reset_req = true;
    }
    */