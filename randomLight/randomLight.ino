#include <stdio.h>
//This script just flashes a light at random intervals + generates a TTL at the
//same time. This is useful for synchronising video and Ephys recording. 

const int ledPin=5;  //pin for the led
const int debugPin=13;  
const int TTLpin=23;
long r_delay;

void setup() {
  Serial.begin(9600);
  pinMode(debugPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  //don't need sep. pin as will TTL from laser trigger
  digitalWrite(debugPin, LOW); 
    digitalWrite(TTLpin, LOW); 
  randomSeed(analogRead(0));  //generate a random seed for the random number generator
}

void loop() {

   
    digitalWrite(ledPin, HIGH); //start low
    digitalWrite(debugPin, HIGH);
    digitalWrite(TTLpin, HIGH);
    
    delay(200); //turn on the light for 100 ms

    digitalWrite(ledPin, LOW); //start low
    digitalWrite(debugPin, LOW);
    digitalWrite(TTLpin, LOW);

    r_delay=random(5000); //random gap between 0-1s
     
    delay(r_delay);
}

    

   
    
