#include <stdio.h>
//this script controls the arduino response to commands from MATLAB.
//This will involve commands to set the laser parameters, activate the laser according to
//information provided, and create TTLS. 
//Overall start and stop for each laser train will come from MATLAB.

const int laserPin=8;  //pin for the laser
const int ledPin=13;   //as a check on the laser, this will light up when the laser does.
//const int debugPin=13; //can connect an LED for debugging purposes.


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
String itis;
String pulseONs;
String numTrain;
char whatStim[ ]="Ready for new parameters - enter inter-trial interval (s) and number of trials";
char theEnd[ ] = "Stimulation Ended";
char theStart[ ] = "Ready, enter 1 to start, enter c to cancel";
char how[]="Ready, enter c to cancel completely or enter 1 to start, then x to stop";
int pulseCount;
int readStage=1;
float iti;
float pulseON;
float tPer=0.0;
float oldF;
float oldPO;
float oldNT;
float numTr;
int ok_go=1;

void setup() {
  Serial.begin(9600);
  pinMode(laserPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  //don't need sep. pin as will TTL from laser trigger
  digitalWrite(laserPin, LOW);
  message==0;
}

void loop() {

if (message){
  Serial.println(whatStim); 
  message=0; 
}


if (Serial.available() && readStage==1) {
  
    digitalWrite(ledPin, LOW); //start low
    digitalWrite(laserPin, LOW);
   
    input  = Serial.readStringUntil(10); //stop reading when it sees newline.  

    //now take apart the string to pull out individual parameters.
    int commaInda = input.indexOf(',');   //find the comma separator  
 
    itis = input.substring(0, commaInda);  //freq in Hz
    numTrain=input.substring(commaInda+1);  //how many times to do this, i.e. keep going until 10 pulses have occured. If set to 0, this will run indefinitely until stopped by user input. 

    
    iti=itis.toFloat();   
    numTr=numTrain.toFloat();
    
    if (iti !=0){
    tPer= 1000.0*iti;  //time period for cycle, in milliseconds
    }
    
    pulseON=20;  //20ms TTL hardwired in

   
    Serial.println("ITI, TTL length, number of TTLs are:");
    Serial.println(iti);
    Serial.println(pulseON);
    if (numTr==0){
    Serial.println("continuous");
    }
    else{
     Serial.println(numTr); 
    }   

      vals=1;  //ready to go, part one
      readStage++;
   
   if (ok_go){
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
    digitalWrite(laserPin, HIGH);
    digitalWrite(ledPin, HIGH);
    delay(pulseON);
    digitalWrite(laserPin, LOW);
    digitalWrite(ledPin, LOW);
    delay(tPer-pulseON); //off for remainder of cycle
    pulseCount++;
   }
   
   else if (cycletype==1 && pulseCount>=numTr){     
      go=0;  // stop, reset everything.
      readStage=1;
      vals=0;
      digitalWrite(laserPin, LOW);
      digitalWrite(ledPin, LOW);
      pulseCount=0;
      Serial.println(theEnd);
      message=1;
      eat  = Serial.readStringUntil(10); //clear out the buffer
   }
         
       
 } 

 if (Serial.available() && cycletype==1 && readStage==3) {    //if stimulation is continuous, listen out for a stop signal. 
   Serial.readBytesUntil(10, incomingData, 2); //waiting for a stop signal
   stopper=incomingData[0];
   if (stopper=='c'){ 
      go=0;  // stop, reset everything.
      readStage=1;
      vals=0;
      digitalWrite(laserPin, LOW);
      digitalWrite(ledPin, LOW);
      Serial.readBytesUntil(10, incomingData, 2);     
      Serial.println(theEnd);
      message=1;
      eat  = Serial.readStringUntil(10); //clear out the buffer
    }
 }
 
  
 
}
 

