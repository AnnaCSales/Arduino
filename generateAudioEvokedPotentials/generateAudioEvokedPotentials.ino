
// Anna Sales 21/04/2016
// Control script for tone generation using an Arduino Due, TTL generation and laser pulses (hard-coded at 10hz, 50ms pulses for 400 ms).
// Used in the audio oddball experiment in conjunction with LC laser stimulation
// The Due is needed as it has a DAC, which is used to generate high frequency tones.
// DDS tone generation is based on a work at rcarduino.blogspot.com.

// These are the clock frequencies available to the Arduino Due timers /2,/8,/32,/128
// 84Mhz/2 = 42.000 MHz
// 84Mhz/8 = 10.500 MHz
// 84Mhz/32 = 2.625 MHz
// 84Mhz/128 = 656.250 KHz
// 
// 44.1Khz = CD Sample Rate

// 42Mhz/44.1Khz = 952.38
// 10.5Mhz/44.1Khz = 238.09 // best fit divide by 8 = TIMER_CLOCK2 and 238 ticks per sample 
// 2.625Hmz/44.1Khz = 59.5
// 656Khz/44.1Khz = 14.88

// 84Mhz/44.1Khz = 1904 instructions per tick

// the phase accumulator points to the current sample in the wavetable
uint32_t ulPhaseAccumulator = 0;
// the phase increment controls the rate at which we move through the wave table
// higher values = higher frequencies
volatile uint32_t ulPhaseIncrement = 0;   // 32 bit phase increment, see below

// full waveform = 0 to SAMPLES_PER_CYCLE
// Phase Increment for 1 Hz =(SAMPLES_PER_CYCLE_FIXEDPOINT/SAMPLE_RATE) = 1Hz
// Phase Increment for frequency F = (SAMPLES_PER_CYCLE/SAMPLE_RATE)*F
//#define SAMPLE_RATE 44100.0
//#define SAMPLES_PER_CYCLE 600
#define SAMPLE_RATE 44100.0
#define SAMPLES_PER_CYCLE 600   
#define SAMPLES_PER_CYCLE_FIXEDPOINT (SAMPLES_PER_CYCLE<<20)        //this is 629145600  (take 600 in binary, stick 20 zeros on the end)
#define TICKS_PER_CYCLE (float)((float)SAMPLES_PER_CYCLE_FIXEDPOINT/(float)SAMPLE_RATE)    //this is 1.4266e+04


//such that the phase increment we need (as a 32bit int) is freq (hz) * TICKS_PER_CYCLE - can hard code this given that we only need 2 frequencies. 
//for 12kHz this is 171192000
//for 11khz this is 156926000
//for 10kHz this is 1.4266e+08
//for 9000Hz this is 1.2840e+08
//for 8000HZ this is 1.1413e+08
//for 7000Hz this is 9.9864e+07
//for 6000Hz this is 8.5598e+07
//for 5000Hz this is 7.1332e+07
//for 4000Hz this is  5.7065e+07
//for 3000Hz this is 4.2799e+07
uint32_t phaseInc[]={42799000, 57065000, 71332000, 85598000, 99864000, 114130000, 128400000, 142660000, 156926000, 171192000};

// to represent 600 we need 10 bits
// Our fixed point format will be 10P22 = 32 bits


// We have 521K flash and 96K ram to play with

// Create a table to hold the phase increments we need to generate midi note frequencies at our 44.1Khz sample rate
int flag=0;
long time1;
char incomingData[2];
int com;
int NLPinA=52;
int NLPinB=53;
int laserPin=45;
int debugPin=9;
long time2;
#define MIDI_NOTES 128
uint32_t nMidiPhaseIncrement[MIDI_NOTES];



// Create a table to hold pre computed sinewave, the table has a resolution of 600 samples
#define WAVE_SAMPLES 600
// default int is 32 bit, in most cases its best to use uint32_t but for large arrays its better to use smaller
// data types if possible, here we are storing 12 bit samples in 16 bit ints
uint16_t nSineTable[WAVE_SAMPLES];

// create the individual samples for our sinewave table
void createSineTable()
{
  for(uint32_t nIndex = 0;nIndex < WAVE_SAMPLES;nIndex++)
  {
    // normalised to 12 bit range 0-4095
    nSineTable[nIndex] = (uint16_t)  (((1+sin(((2.0*PI)/WAVE_SAMPLES)*nIndex))*4095.0)/2);
  //  Serial.println(nSineTable[nIndex]);
  }
}

void setup()
{
  Serial.begin(9600);

  pinMode(NLPinA, OUTPUT);
  pinMode(NLPinB, OUTPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(debugPin, OUTPUT);
  digitalWrite(debugPin, LOW);
  digitalWrite(NLPinA, LOW);
  digitalWrite(NLPinB, LOW);
  digitalWrite(laserPin, LOW);
  digitalWrite(debugPin, LOW);
 // createNoteTable(SAMPLE_RATE);
  createSineTable();
  
  /* turn on the timer clock in the power management controller */
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(ID_TC4);

  /* we want wavesel 01 with RC */
  TC_Configure(/* clock */TC1,/* channel */1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);
  TC_SetRC(TC1, 1, 238); // sets <> 44.1 Khz interrupt rate
  TC_Start(TC1, 1);
  
  // enable timer interrupts on the timer 
  TC1->TC_CHANNEL[1].TC_IER=TC_IER_CPCS;
  TC1->TC_CHANNEL[1].TC_IDR=~TC_IER_CPCS;
  
  /* Enable the interrupt in the nested vector interrupt controller */
  /* TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number (=(1*3)+1) for timer1 channel1 */
  NVIC_EnableIRQ(TC4_IRQn);

  // this is a cheat - enable the DAC, set level to midpoint (where wave will start)
  analogWrite(DAC1,2045);  //this is the middle point of the voltage variation, where each wave cycle starts and ends.


}

void loop()
{
  
   ulPhaseIncrement = 0;    //stop cycling through sin wave table, for now.
    
if (Serial.available() > 0) {
 // read the incoming byte:
  Serial.readBytesUntil(10, incomingData, 2); //10 is the ASCII for LF (the MATLAB terminator), and the 2 is the max size. Store command in incomingData.                                                                                     
  com=incomingData[0]-48;  //convert the number from char to int
  // dacc_write_conversion_data(DACC_INTERFACE, 2045) ;      
  
  if (com==0){  //standard tone (LOW), no laser
    delay(100);
    digitalWrite(NLPinA, HIGH);       
    ulPhaseIncrement =phaseInc[2] ;
    delay(50);
    digitalWrite(NLPinA, LOW);  
    ulPhaseIncrement = 0;
   // delay(200);
 //  dacc_write_conversion_data(DACC_INTERFACE, 2045);  //if the tone has stopped with the voltage away from the centre point, this will reset it
  }

 if (com==1){  //oddball tone (HIGH), no laser
    delay(100);
    digitalWrite(NLPinB, HIGH);       
    ulPhaseIncrement =phaseInc[3] ;
    delay(50);
    digitalWrite(NLPinB, LOW);  
    ulPhaseIncrement = 0;
  //  delay(50);
  //  dacc_write_conversion_data(DACC_INTERFACE, 2045);
  }

    if (com==2){  //standard plus laser
    digitalWrite(laserPin, HIGH);
    delay(50);
    digitalWrite(laserPin, LOW);
    delay(50);
    digitalWrite(laserPin, HIGH);
    digitalWrite(NLPinA, HIGH); 
    ulPhaseIncrement =phaseInc[2]; 
    delay(50);
    digitalWrite(laserPin, LOW);
    digitalWrite(NLPinA, LOW); 
    ulPhaseIncrement = 0;
    delay(50);
    digitalWrite(laserPin, HIGH);
  //  dacc_write_conversion_data(DACC_INTERFACE, 2045);
    delay(50);
    digitalWrite(laserPin, LOW);
    delay(50);
    digitalWrite(laserPin, HIGH);
    delay(50);
    digitalWrite(laserPin, LOW);
   
  }

    if (com==3){  //oddball plus laser
    digitalWrite(laserPin, HIGH);
    delay(50);
    digitalWrite(laserPin, LOW);
    delay(50);
    digitalWrite(laserPin, HIGH);
    digitalWrite(NLPinB, HIGH); 
    ulPhaseIncrement =phaseInc[3];  
    delay(50);
    digitalWrite(laserPin, LOW);
    digitalWrite(NLPinB, LOW); 
    ulPhaseIncrement = 0;
    delay(50);
    digitalWrite(laserPin, HIGH);
  // dacc_write_conversion_data(DACC_INTERFACE, 2045);
    delay(50);
    digitalWrite(laserPin, LOW);
    delay(50);
    digitalWrite(laserPin, HIGH);
    delay(50);
    digitalWrite(laserPin, LOW);
  }

    if (com==4){  //long tone for calibration of volume
   
    ulPhaseIncrement =phaseInc[3]; 
    delay(10000);
    ulPhaseIncrement = 0;
    delay(200);  
    dacc_write_conversion_data(DACC_INTERFACE, 2045);
  }

     if (com==5){  //long tone for calibration of volume
   
    ulPhaseIncrement =phaseInc[2];  ;//nMidiPhaseIncrement[ulInput>>3]; 
    delay(10000);
    ulPhaseIncrement = 0;
    delay(200);  
    dacc_write_conversion_data(DACC_INTERFACE, 2045);
  }
  
  else{
  ulPhaseIncrement = 0;
  
  }

  com=6;
   } 
}


void TC4_Handler()
{
  // We need to get the status to clear it and allow the interrupt to fire again
  TC_GetStatus(TC1, 1);
  
 
   if(ulPhaseIncrement==0)
  {
     dacc_write_conversion_data(DACC_INTERFACE, 2045);
     ulPhaseAccumulator=0;
   }

   else
   {
     ulPhaseAccumulator += ulPhaseIncrement;   // 32 bit phase increment, see below
  // if the phase accumulator over flows - we have been through one cycle at the current pitch,
  // now we need to reset the grains ready for our next cycle
  if(ulPhaseAccumulator > SAMPLES_PER_CYCLE_FIXEDPOINT)
  {
   //  carry the remainder of the phase accumulator
   ulPhaseAccumulator -= SAMPLES_PER_CYCLE_FIXEDPOINT;
   }

  // get the current sample   
  uint32_t ulOutput = nSineTable[ulPhaseAccumulator>>20];
  
  // we cheated and user analogWrite to enable the dac, but here we want to be fast so
  // write directly  
  dacc_write_conversion_data(DACC_INTERFACE, ulOutput);
   }
}
