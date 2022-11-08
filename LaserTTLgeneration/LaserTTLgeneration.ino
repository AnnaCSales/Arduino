#include <stdio.h>

//This script generates TTLs to activate the laser according to parameters set by the user. Upload code and open the Serial monitor.

const int laserPin=3;  //pin for controlling the laser
const int ledPin=31;   //as a check on the laser, this pin will go high when the laser does. 
                       //Useful for debugging when the laser is not attached (connect to an LED or use the Arduino's built in LED)


// declare variables:
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
int pulseCount;
int readStage=1;
float freq;
float pulseON;
float tPer=0.0;
float oldF;
float oldPO;
float oldNT;
float numTr;
int ok_go=1;

void setup() {             //setup variables, begin with everything switched off.
  Serial.begin(9600);
  pinMode(laserPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  
  digitalWrite(laserPin, LOW);
  message==1;
}

void loop() {

if (message){
  Serial.println(whatStim);   //ask user for parameters
  message=0; 
}


if (Serial.available() && readStage==1) {
  
    digitalWrite(ledPin, LOW); //start low
    digitalWrite(laserPin, LOW);
   
    input  = Serial.readStringUntil(10); //get input from user, stop reading when newline is entered.  

    //now take apart the string to pull out individual parameters.
    int commaInda = input.indexOf(',');   //find the first comma separator  
    int commaIndb = input.lastIndexOf(','); //find the last comma separator
    
    freqs = input.substring(0, commaInda);  //freq in Hz
    pulseONs=input.substring(commaInda+1, commaIndb); //length of time each pulse is on
    numTrain=input.substring(commaIndb+1);  //how many times to do this, i.e. keep going until 10 pulses have occured. 
                                            //If set to 0, this will run indefinitely until stopped by user input. 

    numTr=numTrain.toFloat();
    freq=freqs.toFloat();   

    if (numTr==1){
    freq=0.01;
    }

    if (numTr>1){
      if (freq !=0){
       tPer= 1000.0/freq;  //time period for cycle, in milliseconds
      }
    }
    
    pulseON=pulseONs.toFloat(); 
    numTr=numTrain.toFloat();
    
    // Confirm to user what has been selected:
    Serial.println("Freq (hz), pulse length (ms), number of pulses are as follow");
    Serial.println(freq);
    Serial.println(pulseON);
    if (numTr==0){
    Serial.println("continuous");
    }
    else{
     Serial.println(numTr); 
    }   

    vals=1;  //we are now ready to go
    readStage++;  //increment readstage to move on to the next step
   
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
   } 
   else if (start!='1') {
    go=0;
    eat  = Serial.readStringUntil(10); //clear out the buffer  
    readStage=1;
    vals=0;
    message=1;
   }
 
 }
 

 if (vals && go) {     //this generates the TTLs as requested
   
   if (cycletype==1 && pulseCount < numTr) {  
    digitalWrite(laserPin, HIGH);
    digitalWrite(ledPin, HIGH);
    delay(pulseON);
    digitalWrite(laserPin, LOW);
    digitalWrite(ledPin, LOW);
     if (numTr>1){
    delay(tPer-pulseON); //off for remainder of cycle
     }
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
   
   else if (cycletype==2)  //keep going indefinitely if continuous TTLs are required
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
    }
 }
 
  
 
}
 
