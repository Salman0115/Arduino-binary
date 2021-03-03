#include <Wire.h>
#define SWITCH1_BIT B00000001 
#define SWITCH1_PRESSED !(PINB & SWITCH1_BIT)
#define SWITCH1_RELEASED !SWITCH1_PRESSED 

#define SWITCH2_BIT B00000010 
#define SWITCH2_PRESSED !(PINB & SWITCH2_BIT)
#define SWITCH2_RELEASED !SWITCH2_PRESSED

//#define SWITCH1 = 9;
//#define SWITCH2 = 8;

// define your MPU-6050 address here

#define MPU_6050        0x68    // Add I2C address of MPU6050
#define PWR_MGMT_1      0x6B    // Add address of PWR_MGMT_1 register

#define ACCEL_XOUT_HI   0x3B
#define ACCEL_YOUT_HI   0x3D
#define ACCEL_ZOUT_HI   0x3F    // Add ACCEL_ZOUT (MSB) register

#define latchPin  B00001000/*11*/                   
#define clockPin  B00010000/*12*/                   
#define dataPin   B00000100/*10*/                  

// define all pins and 7 seg display images
// define your MPU-6050 address here
// define all pins and 7 seg display images

#define LED_OFF ((unsigned char) (0))
#define LED_BLANK ((unsigned char) (128))
#define LED_ZERO ((unsigned char) (1+2+4+8+16+32))
#define LED_ONE ((unsigned char) (6))
#define LED_TWO ((unsigned char) (1+2+8+16+64))
#define LED_THREE ((unsigned char) (79))
#define LED_FOUR ((unsigned char) (102))
#define LED_FIVE ((unsigned char) (109))
#define LED_SIX ((unsigned char) (125))
#define LED_SEVEN ((unsigned char) (7))
#define LED_EIGHT ((unsigned char) (127))
#define LED_NINE ((unsigned char) (111))
#define LED_A ((unsigned char) (119))
#define LED_b ((unsigned char) (124))
#define LED_C ((unsigned char) (57))
#define LED_d ((unsigned char) (94))
#define LED_E ((unsigned char) (121))
#define LED_F ((unsigned char) (113))
#define LED_L ((unsigned char) (56))
#define LED_l ((unsigned char) (1+32))
#define LED_r ((unsigned char) (64+32))
#define LED_U ((unsigned char)(55))
#define LED_t ((unsigned char) (120))

#define RedLed      B10000000    
#define OrangeLed   B01000000
#define GreenLed    B00100000

#define RedLed2     B00010000        
#define OrangeLed2  B00001000    
#define GreenLed2   B00000100 

int Green1 = 5000;
int Green2 = 5000;

#define MAX_STATES 9
const byte states[MAX_STATES] = {
  
   RedLed | RedLed2,                   ///state 1 
   RedLed | OrangeLed | RedLed2,       ///state 2 
   GreenLed | RedLed2,                 ///state 3 
   OrangeLed | RedLed2,                ///state 4 
   RedLed | RedLed2,                   ///state 5 
   RedLed | RedLed2 | OrangeLed2,      ///state 6 
   RedLed | GreenLed2,                 ///state 7 
   RedLed | OrangeLed2,                ///state 8 

};



// define  some debounce constants in ms

#define debounce 300 //normal debounce duration
#define short_debounce 50 //short debounce duration

// declare the state variable of the 7 segment display
unsigned char ledstate; 

// module0  0 - F button counter
// module0 variables

bool init_module0_clock;
unsigned long module0_time, module0_delay;
bool module0_doStep;
unsigned char module0_i; // state variable for module 0
unsigned char module0_count;

// module1 button 1 debouncer and driver
// module1 variables  
  
bool init_module1_clock;
unsigned long module1_time, module1_delay;
unsigned long debounce_count;
bool module1_doStep;
unsigned char module1_i; // state variable for module 1

// module2 traffic light controller
// module2 variables  
  
#define Leds_OFF       B00000000

bool init_module2_clock;
unsigned long module2_time, module2_delay;
bool module2_doStep;
unsigned char module2_i; // state variable for module 2
unsigned char module2_count; // sec counter for module 2

// check A.8.4 Enumerations on page 175 in Kernighan
// Ritchie for a definition of priority_t as a type
// and the variables module2_priority, module2_new_priority
// check also page 38 in "The C" book for an example

enum priority_t { EQUAL, SET1, SET2 };
priority_t module2_priority, module2_new_priority;

// module3 MPU-6050 driver
// module3 variables   
 
bool init_module3_clock;
unsigned long module3_time, module3_delay;
bool module3_doStep, enableCalm, doCalm;
unsigned char module3_i; // state variable for module 3
int16_t AcX,AcY,AcZ; // accelerometer variables 

// module4 scheduller
// module4 variables  
  
bool init_module4_clock;
unsigned long module4_time, module4_delay;
bool module4_doStep;
unsigned char module4_i; // state variable for module 4
unsigned char module4_1;

// module5  second button driver and debouncer
// module5 variables

bool init_module5_clock;
unsigned long module5_time, module5_delay;
unsigned long debounce_count_B2;
bool module5_doStep;
unsigned char module5_i; // state variable for module 5

// module6  heart beat module
// module6 variables   
 
bool init_module6_clock;
unsigned long module6_time, module6_delay;
bool module6_doStep;
unsigned char module6_i; // state variable for module 6

// definition for the type of the button state variables

enum button_t { notPRESSED, partialPRESS, normalPRESS, heldPRESS,
                preDblClick, partialDblClick, DblClick, afterDblClick };
button_t B1_state; // button B1 state variable
button_t B2_state; // button B2 state variable


void setup() {
  // DATA, CLOCK and LATCH pin for the shift register
  // to be configured as outputs
  
  DDRB |= latchPin ;
  DDRB |= clockPin;
  DDRB |= dataPin ;
  ledstate = B00000000;
  
  // set clock and latch pin to LOW
  togglelatch();
  toggleclock();
  
  // set the state variable for your
  // 7 seg display to "all_segments_off"

  // button 1 and 2 pins to be configured as INPUT_PULLUP
  
  DDRB &= ~SWITCH1_BIT;
  PORTB |= SWITCH1_BIT;
  
  DDRB &= ~SWITCH2_BIT;
  PORTB |= SWITCH2_BIT;
  
  // set the state variables for both
  // buttons to "not pressed"
  
  B1_state = notPRESSED;
  B2_state = notPRESSED;
  
  // all traffic light led pins to be configured
  // as outputs and to be turned off.
  
  DDRD |= B11111100;
  PORTD |= B00000000;
  
  // initialise the clocks on all modules
  
  init_module0_clock = true;
  init_module1_clock = true;
  init_module2_clock = true;
  init_module3_clock = true;
  init_module4_clock = true;
  init_module5_clock = true;
  init_module6_clock = true;
  
  // initialise their variables here as well
  
   Serial.begin(9600);
   
   Wire.begin();
  // initialise the I2C bus. wake up the MPU-6050
    Wire.beginTransmission(MPU_6050);
    Wire.write(PWR_MGMT_1); // Power Management 1 Register
    Wire.write(0); // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
}


void loop() {
  

  { // module 0
    
    if (init_module0_clock) {
      module0_delay = 20;
      module0_time=millis();
      module0_doStep=false;
      init_module0_clock=false;
      module0_i=0;
    }
    else {
      unsigned long m;
      m=millis();
      if (m - module0_time > module0_delay) {
        module0_time = m; module0_doStep = true;
      }
      else module0_doStep = false;
    }

    if (module0_doStep) {
       unsigned char msb = 0b10000000 & ledstate; 
       switch(module0_count){
          case 0:ledstate = msb | LED_ZERO; break;
          case 1:ledstate = msb | LED_ONE;break;
          case 2:ledstate = msb | LED_TWO;break;
          case 3:ledstate = msb | LED_THREE;break;
          case 4:ledstate = msb | LED_FOUR;break;
          case 5:ledstate = msb | LED_FIVE;break;
          case 6:ledstate = msb | LED_SIX;break;
          case 7:ledstate = msb | LED_SEVEN;break;
          case 8:ledstate = msb | LED_EIGHT;break;
          case 9:ledstate = msb | LED_NINE;break;
          case 10:ledstate = msb | LED_A;break;
          case 11:ledstate = msb | LED_b;break;
          case 12:ledstate = msb | LED_C;break;
          case 13:ledstate = msb | LED_d;break;
          case 14:ledstate = msb | LED_E;break;
          case 15:ledstate = msb | LED_F;break;
          default:module0_i = 0; ledstate = msb | LED_OFF;  
       }
       switch (module0_i) {
        case 0: 
          if (B1_state==normalPRESS) {
            module0_i=1;
            module0_count=(module0_count+1)%16;
           
          }
          break;
        case 1:
          if (B1_state==notPRESSED) {
            module0_i=0;
          }
          break;
        default: module0_i=0;
       }
        
    }
  }


  { // module 1
    
    if (init_module1_clock) {
      module1_delay = 17;
      module1_time=millis();
      module1_doStep=false;
      init_module1_clock=false;
      module1_i=0;
    }
    else {
      unsigned long m;
      m=millis();
      if (m - module1_time > module1_delay) {
        module1_time = m; module1_doStep = true;
      }
      else module1_doStep = false;
    }

    if (module1_doStep) {
      switch(module1_i){
          case 0: 
            B1_state=notPRESSED;
            debounce_count = module1_time;
            if (SWITCH2_RELEASED) module1_i = 0;
            else module1_i = 1;
            break;
          case 1: 
            B1_state=partialPRESS;
            if (SWITCH2_RELEASED) module1_i = 0;
            else if (millis()-debounce_count < debounce) module1_i = 1;
            else module1_i = 2;
            break;
          case 2: 
            B1_state=normalPRESS;
            if (SWITCH2_RELEASED) module1_i = 0;
            else if (millis()-debounce_count < 1000) module1_i = 2;
            else module1_i = 3;
            break;
          case 3:
            B1_state==heldPRESS;
            if(SWITCH2_RELEASED)module1_i=0;
            else module1_i=3;
            break;
          // ... etc
          default: module1_i = 0; break;
        }
    }
  }

  { // module 2
    
    if (init_module2_clock) {
      module2_delay = 500;
      module2_time=millis();
      module2_doStep=false;
      init_module2_clock=false;
      module2_i= 10; module2_count=0;
    }
    else {
      unsigned long m;
      m=millis();
      if (m - module2_time > module2_delay) {
        module2_time = m; module2_doStep = true;
      }
      else module2_doStep = false;
    }

    if (module2_doStep) {
      switch(module2_i){
          case 10: // attention yellow || yellow    
             PORTD = OrangeLed | OrangeLed2;
             module2_i=0;
             module2_delay = 3000;
            break;
          case 0: // =red ||red
             PORTD =  RedLed | RedLed2,
             module2_i=1;
             module2_delay = 1000;
            break;
          case 1: // =red, yellow || red
             PORTD = states[module2_i];
             module2_i= 2;
             module2_delay = 1000;
            break;
          case 2: // =green || red
             PORTD = states[module2_i];
             module2_i=3;
             module2_delay = Green1;
            break;
          case 3: // =yellow || red
             PORTD = states[module2_i];
             module2_i=4;
             module2_delay = 1000;
            break;
          case 4: // =red ||red
             PORTD = states[module2_i];
             module2_i=5;
             module2_delay = 1000;
            break;
          case 5: // =red || red, yellow
             PORTD = states[module2_i];
             module2_i=6;
             module2_delay = 1000;
            break;
          case 6: // =red || green
             PORTD = states[module2_i];
             module2_i=7;
             module2_delay = Green2;
            break;
          case 7: // =red || yellow
             PORTD = states[module2_i];
             module2_i=0;
             module2_delay = 1000;
            break;
          default:module2_i = 10;break;
        }
    }
  }

  { // module 3
    
    if (init_module3_clock) {
      module3_delay = 333; // sample every 333ms as per spec
      module3_time=millis();
      module3_doStep=false;
      init_module3_clock=false;
      module3_i=0;
      Wire.beginTransmission(MPU_6050);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_6050,6,true);  // request a total of 6 registers
      AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
      AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    }
    else {
      unsigned long m;
      m=millis();
      if (m - module3_time > module3_delay) {
        module3_time = m; module3_doStep = true;
      }
      else module3_doStep = false;
    }

    if (module3_doStep) {

      Wire.beginTransmission(MPU_6050);
      Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_6050,6,true);  // request a total of 6 registers
      AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
      AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
      AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
      
      // calculate orientation here
    
     if (AcX < -13000)
    { 
      module3_i = 2;
      //Data = 56; // landscape
    }
    
    if ( AcY > 12000 )
    { 
      module3_i = 4;
      //Data = 33; // left
    }

   if ( AcY < -12000  )
    { 
      module3_i = 5;
      //Data = 96; // right
    }

    if (AcX > 12000 )
    { 
      module3_i = 3;
      //Data =55; // upside down (landscape)   
    }
    if(AcZ >12000)
    {
     module3_i = 0;
     //Data = 113; // flat
    }
    if(AcZ <-10000)
    {
      module3_i = 1;
      //Data = 124;
    }
          
      unsigned char msb = 0b10000000 & ledstate; 
      switch(module3_i){
          case 0:ledstate = msb | LED_F; Green1 = 5000; Green2 = 5000;  break; // flat faced up
          case 1:ledstate = msb | LED_b; Green1 = 5000; Green2 = 5000;  break; // flat faced down
          case 2:ledstate = msb | LED_U; Green1 = 5000; Green2 = 5000;  break; // landscape upside and down
          case 3:ledstate = msb | LED_L; Green1 = 5000; Green2 = 5000;  break; // for landscape
          case 4:ledstate = msb | LED_r; Green1 = 3000; Green2 = 7000; break; // for portrate right
          case 5:ledstate = msb | LED_l; Green1 = 7000; Green2 = 3000; break; // for portrate left
          default:module3_i = 0; ledstate = msb | LED_OFF;
       }
    }
  }

  { // module 4
    
    if (init_module4_clock) {
      module4_delay = 10;
      module4_time=millis();
      module4_doStep=false;
      init_module4_clock=false;
      module4_i=0;
    }
    else {
      unsigned long m;
      m=millis();
      if (m - module4_time > module4_delay) {
        module4_time = m; module4_doStep = true;
      }
      else module4_doStep = false;
    }

    if (module4_doStep) {
      switch (module4_i) {
        case 0: 
          init_module2_clock = false; // run traffic light controller
          init_module0_clock = true; // stop button count
          ledstate = (((unsigned char) (0b10000000 & ledstate))) | LED_t;
          init_module3_clock = true; // keep resetting the gyro
          //if (B1_state==afterDblClick) module4_i = 1;
         // if (B2_state==normalPRESS) module4_i = 1;
          break;
        case 1: 
          init_module2_clock = true; // run traffic light controller
          init_module0_clock = false; // stop button count
          init_module3_clock = true; // keep resetting the gyro
          PORTD=Leds_OFF;
        //  if (B2_state==normalPRESS) module4_i = 2; // button count only mode
          break;
        case 2: 
          init_module2_clock = true; // run traffic light controller
          init_module0_clock = true; // stop button count
          init_module3_clock = false; // keep resetting the gyro
         // if (B2_state==normalPRESS) module4_i = 3; // button count only mode
          break;
        case 3:
          init_module2_clock = false; // run traffic light controller
          init_module0_clock = false; // stop button count
          init_module3_clock = true; // keep resetting the gyro
         // if (B2_state==normalPRESS) module4_i = 4; // button count only mode
          break;
        case 4:
          init_module2_clock = false; // run traffic light controller
          init_module0_clock = true; // stop button count
          init_module3_clock = false; // keep resetting the gyro
         // if (B2_state==normalPRESS) module4_i = 0; // button count only mode
          break;

        
        default: module4_i = 0;
      }
     
    }
     switch (module4_1) {
        case 0: 
          if (B2_state==normalPRESS) {
            module4_1=1;
            module4_i=(module4_i+1)%5;
            
          }
          break;
        case 1:
          if (B2_state==notPRESSED) {
            module4_1=0;
            
          }
          break;
        default: module4_1=0;
       }
  }
  
  { // module 5
    
    if (init_module5_clock) {
      module5_delay = 17;
      module5_time=millis();
      module5_doStep=false;
      init_module5_clock=false;
      module5_i=0;
    }
    else {
      unsigned long m;
      m=millis();
      if (m - module5_time > module5_delay) {
        module5_time = m; module5_doStep = true;
      }
      else module5_doStep = false;
    }

    if (module5_doStep) {
      switch(module5_i){
          case 0: 
            B2_state=notPRESSED;
            debounce_count_B2 = module5_time;
            if (SWITCH1_RELEASED) module5_i = 0;
            else module5_i = 1;
            break;
          case 1: 
            B2_state=partialPRESS;
            if (SWITCH1_RELEASED) module5_i = 0;
            else if (millis()-debounce_count_B2 < debounce) module5_i = 1;
            else module5_i = 2;
            break;
          case 2: 
            B2_state=normalPRESS;
            if (SWITCH1_RELEASED) module5_i = 0;
            else if (millis()-debounce_count_B2 < 1000) module5_i = 2;
            else module5_i = 3;
            break;
          case 3:
            B2_state=heldPRESS;
            if(SWITCH1_RELEASED)module5_i=0;
            else module5_i=3;
            break;
          // ... etc
          default: module5_i = 0; break;
        }
    }
  }

  { // module 6
    
    if (init_module6_clock) {
      module6_delay = 320;
      module6_time=millis();
      module6_doStep=false;
      init_module6_clock=false;
      module6_i=0;
    }
    else {
      unsigned long m;
      m=millis();
      if (m - module6_time > module6_delay) {
        module6_time = m; module6_doStep = true;
      }
      else module6_doStep = false;
    }

    if (module6_doStep) {
       switch (module6_i) {
        case 0: 
          ledstate = 0b01111111 & ledstate; 
          module6_i = 1;
          break;
        case 1:
          ledstate = 0b10000000 | ledstate; 
          module6_i=0;
          break;
        default: module6_i=0;
       } 
    }
  }

  shiftByteMSF(ledstate); 
}

void shiftBit(bool data) {

PORTB = dataPin & data;
    toggleclock();
}

void toggleclock() {

 PORTB |= clockPin;
 PORTB &= ~clockPin;
 
}
void togglelatch() {

 PORTB |= latchPin;
 PORTB &= ~latchPin;
}

void shiftByteMSF(unsigned char b) {

unsigned char m;                             
for(m=128; m>0; m=m>>1)
{
    if (b & m)
        PORTB |= dataPin;
    else
        PORTB &= ~dataPin;
    toggleclock();

}
togglelatch();
}
