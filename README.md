# Arduino-Flex-Fuel
Arduino based flex fuel signal converter 
This program will sample a 50-150hz signal depending on ethanol content, and output a 0-5V signal via PWM.
The LCD will display ethanol content, hz input, mv output, fuel temp

Connect PWM output to NEMU Breakoutboard on ADC0-3 (or your engine management of choice), and tune the "FLEX FUEL SETUP" tab accordingly. NOTE: Lowpass filter to be used on output.

Input pin 8 (PB0) ICP1 on Atmega328
Output pin 3 PWM

If LCD Keypad shield is used, solder jumper from Pin 8 - Pin 2,
and snip leg from pin 8 (docs folder)
