// Modded from RobG's LCD Shift Reg Code

#include "msp430g2553.h"

#ifndef LCD_H_
#define LCD_H_

#define sendData(data) send(data, 1)
#define sendInstruction(data) send(data, 0)
#define initDisplay() sendInstruction(0x3C); sendInstruction(0x0C); clearDisplay(); sendInstruction(0x06)
#define clearDisplay() sendInstruction(0x01); _delay_cycles(2000)
#define second_line() sendInstruction((0x80 | 0x40)); _delay_cycles(2000)
#define DATAPIN BIT4
#define CLOCKPIN BIT5
#define ENABLEPIN BIT3
#define test_line() sendInstruction((0x80 | 0xB | 0x40)); _delay_cycles(2000)


void sendDataArray(char data[], char length);
void send(char data, char registerSelect);
void Print_Screen(char *texts);


#endif /*LCD_H_*/
