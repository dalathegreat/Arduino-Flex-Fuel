/*******************************************************
    ________    _______  __    ________  __________ 
   / ____/ /   / ____/ |/ /   / ____/ / / / ____/ / 
  / /_  / /   / __/  |   /   / /_  / / / / __/ / /  
 / __/ / /___/ /___ /   |   / __/ / /_/ / /___/ /___
/_/   /_____/_____//_/|_|  /_/    \____/_____/_____/

v1.0 - Daniel Öster

This program will sample a 50-150hz signal depending on ethanol 
content, and output a 0-5V signal via PWM.
The LCD will display ethanol content, hz input, mv output, fuel temp

Connect PWM output to NEMU Breakoutboard on ADC0-3, and tune
the "FLEX FUEL SETUP" tab accordingly. NOTE: Lowpass filter to
be used on output.

Input pin 8 (PB0) ICP1 on Atmega328
Output pin 3 PWM

If LCD Keypad shield is used, solder jumper from Pin 8 - Pin 2,
and snip leg from pin 8 http://i.imgur.com/KdlLmye.png
********************************************************/

// include the library code:
#include <LiquidCrystal.h> //LCD plugin

// initialize the library with the numbers of the interface pins 
LiquidCrystal lcd(2, 9, 4, 5, 6, 7); //LCD Keypad Shield

int inpPin = 8;     //define input pin to 8
int outPin = 3;    //define PWM output, possible pins with LCD are 3, 10 and 11 (UNO)

//Define global variables
volatile uint16_t revTick;    //Ticks per revolution
uint16_t pwm_output  = 0;     //integer for storing PWM value (0-255 value)
int HZ = 0;                   //unsigned 16bit integer for storing HZ input
int ethanol = 0;              //Store ethanol percentage here

int duty;           //Duty cycle (0.0-100.0)
float period;       //Store period time here (eg.0.0025 s)
float temperature = 0;  //Store fuel temperature here
int fahr = 0;
int cels = 0;
static long highTime = 0;
static long lowTime = 0;
static long tempPulse;

void setupTimer()   // setup timer1
{           
  TCCR1A = 0;      // normal mode
  TCCR1B = 132;    // (10000100) Falling edge trigger, Timer = CPU Clock/256, noise cancellation on
  TCCR1C = 0;      // normal mode
  TIMSK1 = 33;     // (00100001) Input capture and overflow interupts enabled
  TCNT1 = 0;       // start from 0
}

ISR(TIMER1_CAPT_vect)    // PULSE DETECTED!  (interrupt automatically triggered, not called by main program)
{
  revTick = ICR1;      // save duration of last revolution
  TCNT1 = 0;       // restart timer for next revolution
}

ISR(TIMER1_OVF_vect)    // counter overflow/timeout
{ revTick = 0; }        // Ticks per second = 0


void setup()
{
  pinMode(inpPin,INPUT);
  setPwmFrequency(outPin,1); //Modify frequency on PWM output
 setupTimer();
   // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Initial screen formatting
  lcd.setCursor(0, 0);
  lcd.print("Ethanol:    %");
  lcd.setCursor(0, 1);
  lcd.print("     Hz       C");
}
 
void loop()
{
  getfueltemp(inpPin); //read fuel temp from input duty cycle
  
  if (revTick > 0) // Avoid dividing by zero, sample in the HZ
    {HZ = 62200 / revTick;}     // 3456000ticks per minute, 57600 per second 
    else                        // 62200 per seconmd seems to be more accurate? 
    {HZ = 0;}                   //needs real sensor test to determine correct tickrate

  //calculate ethanol percentage
    if (HZ > 50) // Avoid dividing by zero
    {ethanol = HZ-50;}
    else
    {ethanol = 0;}

if (ethanol > 99) // Avoid overflow in PWM
{ethanol = 99;}

  //Screen calculations
  pwm_output = 255 * (ethanol*0.01); //calculate output PWM for NEMU
  
  lcd.setCursor(10, 0);
  lcd.print(ethanol);
  
  lcd.setCursor(2, 1);
  lcd.print(HZ);
  
  lcd.setCursor(8, 1); 
  lcd.print(temperature); //Use this for celsius

  //PWM output
  analogWrite(outPin, pwm_output); //write the PWM value to output pin

  delay(100);  //make screen more easily readable by not updating it too often
  
}

void getfueltemp(int inpPin){ //read fuel temp from input duty cycle
highTime = 0;
lowTime = 0;

tempPulse = pulseIn(inpPin,HIGH);
  if(tempPulse>highTime){
  highTime = tempPulse;
  }

tempPulse = pulseIn(inpPin,LOW);
  if(tempPulse>lowTime){
  lowTime = tempPulse;
  }

duty = ((100*(highTime/(double (lowTime+highTime))))); //Calculate duty cycle (integer extra decimal)
float T = (float(1.0/float(HZ)));               //Calculate total period time
float period = float(100-duty)*T;               //Calculate the active period time (100-duty)*T
float temp2 = float(10) * float(period);        //Convert ms to whole number
temperature = ((40.25 * temp2)-81.25); // Calculate temperature for display (1ms = -40, 5ms = 80)
int cels = int(temperature);
cels = cels*0.1;
float fahrtemp = ((temperature*1.8)+32);
fahr = fahrtemp*0.1;

}

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
