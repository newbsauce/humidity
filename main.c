// Reflow Oven MSP430:
// Code By Natahn Zimmerman
// 12/17/11

/////////////Includes///////////////////
#include "stdbool.h"
#include "msp430g2453.h"
#include "lcd.h"
#include "msp430.h"

////////////ARB Defines///////////////////



/////////////Micro Defined////////////////

// PORT 1.0
#define TX			BIT1	//Uart TX
#define RX			BIT2	//Uart RX
#define Bat_Level	BIT3	//Analog for battery power
#define USB_PW		BIT4	//Digital for USB power
#define SCLK        BIT5   	//SPI CLK Master G2553
#define MOSI		BIT7	//SPI Master MOSI
#define MISO		BIT6	//SPI Master MISO

// PORT 2.0

#define nLCD			BIT0	//LCD Backlight active low
#define CS				BIT1	//SPI Master CS
#define Status			BIT2
#define ENABLEPIN       BIT3   // LCD Enable Pin
#define DATAPIN			BIT4   // LCD Data Pin (Shift Reg)
#define CLOCKPIN     	BIT5   // LCD Clock Pin (Shift Reg)
#define SW1      		BIT6    //CNTRL SW 1 // Start/Stop
#define SW2            	BIT7   //CNTRL SW 2 // RESET


//////////Other Micro Defines///////////

char gspibuf[3] ={0,0,0};
char Gtx_buffer[2];
uint16_t spi_data_global = 0;


//////////////Variables/////////////////

//////////////PROTOS////////////////////

//////New PROTOS/////////////////////////

void welcome_message();

void gpio_setup();

uint16_t get_spi_data();

void hex_to_char(uint16_t data, char *cdata);

//////////////Main//////////////////////

void main(void) {

	char tx_buffer[5];


	msp430_clk_setup();			//1MHZ + stop WDT


	gpio_setup();
	mps430_uart_setup();
	msp430_spi_setup();
	welcome_message();

	uart_sends("\n");
	uart_sends("Meter Power up Finished. In button loop\r\n");

	P2DIR &=~BIT1;
	P2REN |=BIT1;
	P2OUT |= BIT1;

	//Test Code here
	hex_to_char(0xD,tx_buffer);
	uart_sends(tx_buffer);

	//End of Test Code






	while(1) {

		if((P2IN & SW1)==0)		//Wait for user to press the start button
		{
			break;
		}
	}

	uart_sends("Starting Data Collection\r\n");

	clearDisplay(); // Clear Display
	Print_Screen("Peak: 0000 mW");
	second_line();
	Print_Screen("Latest: 0000 mW");

	msp430_set_timer_A(10000); // Set Timer A to interrupt every 10ms.

	while(1) {
		delay_ms(100);
		hex_to_char(spi_data_global,tx_buffer);
		uart_sends(tx_buffer);
		uart_sends(" \r\n");

		/*
		if((P2IN & BIT1)==BIT1) {
					P2OUT &=~Status;
				}
				else {
					P2OUT |= Status;
				}
		*/
	}



} // End main

void welcome_message() {


	_delay_cycles(100000); // magical delay to make lcd screen work
	initDisplay(); // Turn on display.
	clearDisplay(); // Clear Display
	P2OUT &= ~nLCD;
	_delay_cycles(1000000);
	Print_Screen("Zallus Laser");
	second_line();
	Print_Screen("Power Meter V1.0");
	_delay_cycles(4000000);
	clearDisplay(); // Clear Display
	Print_Screen("Press Start");
	second_line();
	Print_Screen("hi");
	P2OUT &= ~Status;
}

void gpio_setup() {

	P2OUT &= ~(CLOCKPIN + DATAPIN); //Set these pins low (ensures low when defined as output)
	P2OUT |= ENABLEPIN; // Set enable pin high. (ensures high when defined as output)
	P2DIR |= ENABLEPIN + CLOCKPIN + DATAPIN + nLCD + Status; //+SCLK + CS // Define as outputs

	P2OUT |= CS+ nLCD; // Enable CS Starts High;

	switch_setup();

}

uint16_t get_spi_data() {

	uint16_t data=0;
	data = gspibuf[0] << 8;
	data = data + gspibuf[1];
	return data;

}

void hex_to_char(uint16_t func_data, char *cdata) {

	uint16_t data = func_data;
	uint16_t byte_data = 0;
	uint8_t i = 0 ;

	for(i=0; i<4; i++) {
		byte_data = 0;
		byte_data = (data & (0xF<<(4*i))) >> (4*i);
		cdata[3-i] ="0123456789ABCDEF"[byte_data & 0x0F];

	}
	cdata[4] = 0;

}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	static char tx_buffer[2];
	static bool second_byte=0;
	static bool third_byte=0;
	static uint8_t adc_first_byte = 0;
	static uint8_t adc_second_byte=0;
	uint8_t adc_data = 0;

	if((IFG2 & UCA0RXIFG)) // IF UART DATA
	{
		tx_buffer[0] = UCA0RXBUF;

		if (tx_buffer[0]=='1') {
			clearDisplay();
			sys_reset();
		}

	}

	if((IFG2 & UCB0RXIFG)) // IF SPI DATA
	{
		adc_data = UCB0RXBUF;

		if(adc_data == 11) {
			second_byte = true;
		}

		if(second_byte) {
			second_byte = false;
			third_byte = true;
			adc_first_byte = adc_data;

		}

		if(third_byte) {
			third_byte = false;
			adc_second_byte = adc_data;
			spi_data_global = (adc_first_byte)<<8 + adc_second_byte;
			adc_first_byte = 0;
			adc_second_byte = 0;
			P2OUT ^= Status;
		}
	}


}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {
	const uint8_t arb_data1[1] = {0x0B}; //Dummy Data
	spi_sends((uint8_t *)arb_data1, 1, BIT7); // Send SPI to
	spi_sends((uint8_t *)arb_data1, 1, BIT7); // Send SPI to
	spi_sends((uint8_t *)arb_data1, 1, BIT7); // Send SPI to

}










/*
void get_temp() {

	heat_data = 0; // Reset heat data to zero
	buffer = 0;

	P1OUT &= ~CS; // Bring chip select line low

	while ((UCA0STAT & UCBUSY))
		;

//Send 4 dummy bytes of SPI, collect first 16 bytes of data

	UCA0TXBUF = 0x1; // Dummy Byte
	while (!(IFG2 & UCA0TXIFG))
		; // Wait until RX is done
	while ((UCA0STAT & UCBUSY))
		;
	heat_data = (buffer << 8); //shift msb data over
	UCA0TXBUF = 0x1; // Dummy Byte
	while (!(IFG2 & UCA0TXIFG))
		; // Wait until RX is done
	while ((UCA0STAT & UCBUSY))
		;
	heat_data |= buffer;
	UCA0TXBUF = 0x1; // Dummy Byte
	while (!(IFG2 & UCA0TXIFG))
		; // Wait until RX is done
	while ((UCA0STAT & UCBUSY))
		;
	UCA0TXBUF = 0x1; // Dummy Byte
	while (!(IFG2 & UCA0TXIFG))
		; // Wait until RX is done
	while ((UCA0STAT & UCBUSY))
		;

	P1OUT |= CS;

} // end get temp

void display_temp() {

	heat_data = heat_data >> 2;

	heat_data = heat_data / 4;

	ctemp = heat_data;

	temp[0] = (heat_data / 100) + '0';
	heat_data = (heat_data % 100);
	temp[1] = (heat_data / 10) + '0';
	heat_data = (heat_data % 10);
	temp[2] = (heat_data) + '0';
	temp[3] = 'C';
	temp[4] = '\0';

	Print_Screen(temp);

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

	buffer = UCA0RXBUF;

}

*/

/*
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void) {
	TAIV = TAIVs;
	P2OUT ^= ALARM;

}

*/



