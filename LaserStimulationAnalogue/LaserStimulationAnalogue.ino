//This script generates TTLs to control a laser using the analogue controller, which 
//sets the laser power according input voltage. Uses the Adafruit DAC to generate the 
//appropriate analogue output.


#include <stdio.h>
#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

const int laserPin=3;  //pin for the laser
const int ledPin=22;   //as a check on the laser, this will light up when the laser does.
//const int debugPin=13; //can connect an LED for debugging purposes.


int i = 0;
int sample;
int cycletype;
char bufferClear[4];
char incomingData[2];
char moreData[2];
char ender='x';  
char test[]="test error";
char start;
char stopper;
int count=0;
int message=1;
boolean go=0;
boolean vals=0;
String input;   // freq in hz, pulse duration in ms, number of pulses
String eat;   //used to clear the buffer
String freqs;
String pulseONs;
String numTrain;
char whatStim[ ]="Ready for new parameters";
char theEnd[ ] = "Stimulation Ended";
char theStart[ ] = "Ready, enter 1 to start, enter c to cancel";
char how[]="Ready, enter c to cancel completely or enter 1 to start, then x to stop";
int pulseCount=0;
int readStage=1;
float freq;
float pulseON;
float tPer=0.0;
float oldF;
float oldPO;
float oldNT;
float numTr;
int ok_go=1;
boolean allDone=0;
uint32_t counter;
 

void setup() {
  Serial.begin(9600);
   dac.begin(0x62);
  pinMode(laserPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  //don't need sep. pin as will TTL from laser trigger
  digitalWrite(laserPin, LOW);
  message==0;
   dac.setVoltage(0, false);

}

void loop() {

if (message){
  Serial.println(whatStim); 
  message=0; 
}


if (Serial.available() && readStage==1) {
    dac.setVoltage(0, false);
    digitalWrite(ledPin, LOW); //start low
    digitalWrite(laserPin, LOW);
   
    input  = Serial.readStringUntil(10); //stop reading when it sees newline.  

    //now take apart the string to pull out individual parameters.
    int commaInda = input.indexOf(',');   //find the first comma separator  
    int commaIndb = input.lastIndexOf(','); //find the last comma separator
    
    freqs = input.substring(0, commaInda);  //freq in Hz
    pulseONs=input.substring(commaInda+1, commaIndb); //length of time each pulse is on
    numTrain=input.substring(commaIndb+1);  //how many times to do this, i.e. keep going until 10 pulses have occured. If set to 0, this will run indefinitely until stopped by user input. 

    numTr=numTrain.toFloat();
    
    if (numTr==1){
    freq=0.01;
    }
    else{
    freq=freqs.toFloat();   
    }

    
    if (freq !=0){
    tPer= 1000.0/freq;  //time period for cycle, in milliseconds
    }
  
    pulseON=pulseONs.toFloat(); 
 
   
    Serial.println("Freq (hz), pulse length (ms), number of pulses are as follow");
    Serial.println(freq);
    Serial.println(pulseON);
    if (numTr==0){
    Serial.println("continuous");
    }
    else{
     Serial.println(numTr); 
    }   

      vals=1;  //ready to go, part one
      readStage++;
   
   if (numTr==0 && ok_go){
      cycletype=2;    //if pulseON / freq is set to zero assume constant stimulation is required.
      Serial.println(how);
    }
    else if (numTr>0 && ok_go) {
      cycletype=1;  //otherwise on for pulseON ms at frequency dictated by 'freq'
      Serial.println(theStart);
    }


 }


 if (Serial.available() && readStage==2) {
   Serial.readBytesUntil(10, incomingData, 2); //waiting for the go signal, 2bytes because of LF terminator
   start=incomingData[0];
   if (start=='1') {
    go=1;
   readStage++;
  //  Serial.println("saw 1");
   } 
   else if (start!='1') {
    go=0;
    eat  = Serial.readStringUntil(10); //clear out the buffer  
    readStage=1;
    vals=0;
    message=1;
   }
 
 }
 

 if (vals && go) {   
   
   if (cycletype==1 && pulseCount < numTr) {  
    Serial.println("got to next bit");
    dac.setVoltage(4095, false);;
    digitalWrite(ledPin, HIGH);
    delay(pulseON);
    digitalWrite(ledPin, LOW);
    if (numTr>1){
         Serial.println("numTR >1");
         delay(tPer-pulseON); //off for remainder of cycle
   }
    pulseCount++;
    
        
   }
   
   else if (cycletype==1 && pulseCount>=numTr){  
       
       for (counter = 4095; counter > 15; counter=counter-10)
    {
      dac.setVoltage(counter, false);
      delay(20)
    }
                  
    allDone = 1;
    dac.setVoltage(0, false);

      
      if (allDone) {
      allDone=0;
      go=0;  // stop, reset everything.
      readStage=1;
      vals=0;
      digitalWrite(laserPin, LOW);
      digitalWrite(ledPin, LOW);
      pulseCount=0;
      Serial.println("here");
      message=1;
      eat  = Serial.readStringUntil(10); //clear out the buffer
 
      }
   }
   
   else if (cycletype==2)  //keep going indefinitely
   { 
   
    digitalWrite(laserPin, HIGH);
    digitalWrite(ledPin, HIGH);
    delay(pulseON);
    digitalWrite(laserPin, LOW);
    digitalWrite(ledPin, LOW);
    delay(tPer-pulseON); //off for remainder of cycle     
    }      
       
 } 

 if (Serial.available() && cycletype==2 && readStage==3) {    //if stimulation is continuous, listen out for a stop signal. 
   Serial.readBytesUntil(10, incomingData, 2); //waiting for a stop signal
   stopper=incomingData[0];
   if (stopper=='x'){ 
      go=0;  // stop, reset everything.
      readStage=1;
      vals=0;
      digitalWrite(laserPin, LOW);
      digitalWrite(ledPin, LOW);
      Serial.readBytesUntil(10, incomingData, 2);     
      Serial.println(theEnd);
      message=1;
      eat  = Serial.readStringUntil(10); //clear out the buffer
     dac.setVoltage(0, false);
    }
 }
 
  
 
}
 
