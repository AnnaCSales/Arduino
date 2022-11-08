## Arduino code for ephys
A collection of Arduino files used in experiments in the Pickering lab to trigger lasers, footshocks, and lights. These were created for the Arduino Uno using the Arduino IDE 1.8.19. 

### LaserTTLgeneration 
This code controls a laser by generating TTLs (square wave voltage on-off signal) at user-defined frequency, pulse durations and number of pulses. 
This was originally used in optogenetics experiments, and the TTL output was copied to the openEphys analog input board using a BNC splitter / cable.
The parameters for the TTLs are set using the Arduino Serial monitor once the code is uploaded.

### footshockTrigger
This code is a simpler version of LaserTTLgeneration which simply sends a 50ms TTL at a user defined frequency/number of pulses. Originally used to control 
a Digitimer constant current generator. 

### ObrejaAll
This code was used to deliver the 'Obreja protocol' (Obreja et al 2010, and used in Sales et al 2022) in conjunction with a Digitimer Constant Current stimulator.

### randomLight
Generates a pattern of random flashes of TTLs which are sent to two different pins. Originally used to send out TTLs to openEphys recording at the same time as 
recording rat behaviour on a camera - the random flashes were used to sync the video with the ephys


