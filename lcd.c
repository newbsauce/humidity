#include "lcd.h"

void sendDataArray(char data[], char length) {
	char charIndex = 0;
	charIndex = 0;
     while(charIndex < length) {
        sendData(data[charIndex]);
        charIndex++;
     }
}

void send(char data, char registerSelect) {
	char bitCounter = 0;
	bitCounter = 0;
   while(bitCounter < 8) {
        (data & BIT7) ? (P2OUT |= DATAPIN) : (P2OUT &= ~DATAPIN);
        data <<= 1;
        P2OUT |= CLOCKPIN;
        P2OUT &= ~CLOCKPIN;
        bitCounter++;
     }
     registerSelect ? (P2OUT |= DATAPIN) : (P2OUT &= ~DATAPIN);
     P2OUT &= ~ENABLEPIN;
     P2OUT |= ENABLEPIN;
}


void Print_Screen(char *texts)
{
volatile int i=0;
char *h;
h=texts;
for(i=0;i<16;i++)
{
if(h[i]==0)
   {
   break;
   }
}
sendDataArray(h, i);
}
