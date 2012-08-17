//Humidity Sensor
//For msp430

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

char Gtx_buffer[2];
uint16_t global_adc_data = 0;
uint16_t test_buffer = 0;
static bool sync_complete = false;
char g_buffer[4] = { 0, 0, 0, 0 };
bool start_data_collect = false;
bool startup = false;

//////////////Variables/////////////////

//////////////PROTOS////////////////////

//////New PROTOS/////////////////////////

void welcome_message();

void gpio_setup();

void hex_to_char(uint16_t data, char *cdata);
void data_to_mw(uint16_t data, char *cdata);
void data_to_pc(uint16_t data, char *cdata);

void spi_link_init();
void spi_send_packet();
void update_screen();

//////////////Main//////////////////////

void main(void) {

	msp430_clk_setup(); //1MHZ + stop WDT
	gpio_setup();
	mps430_uart_setup();
	msp430_spi_setup();
	welcome_message();

	uart_sends("\n");
	uart_sends("Humidity_started\r\n");

	//Test Code here

	//End of Test Code

	while (1) {

		if(start_data_collect) {

			break;
		}
	}

	uart_sends("Starting Data Collection\r\n");

	clearDisplay(); // Clear Display
	Print_Screen("Peak: 0000 mW");
	second_line();
	Print_Screen("Latest: 0000 mW");

	_delay_cycles(100);
	spi_link_init();

	msp430_set_timer_A(10000); // Set Timer A to interrupt every 500ms.

	while (1) {

		spi_send_packet();

	}

} // End main

void welcome_message() {

	_delay_cycles(1000000); // magical delay to make lcd screen work
	initDisplay(); // Turn on display.
	clearDisplay(); // Clear Display
	P2OUT &= ~nLCD;
	_delay_cycles(1000000);
	Print_Screen("Rain Samples");
	second_line();
	Print_Screen("Humidity");
	_delay_cycles(4000000);
	clearDisplay(); // Clear Display
	Print_Screen("Press Start");
	second_line();
	Print_Screen("To Begin");
	P2OUT &= ~Status;

}

void gpio_setup() {

	P2OUT &= ~(CLOCKPIN + DATAPIN); //Set these pins low (ensures low when defined as output)
	P2OUT |= ENABLEPIN; // Set enable pin high. (ensures high when defined as output)
	P2DIR |= ENABLEPIN + CLOCKPIN + DATAPIN + nLCD + Status; //+SCLK + CS // Define as outputs

	P2OUT |= CS + nLCD; // Enable CS Starts High;

	P2IE |= BIT6 + BIT7;                             // P interrupt enabled
	P2IES |= BIT6 + BIT7;                            // P Hi/lo edge
	P2IFG &= ~(BIT6 + BIT7);                           // P IFG cleared

	switch_setup();

}

void hex_to_char(uint16_t func_data, char *cdata) {

	uint16_t data = func_data;
	uint16_t byte_data = 0;
	uint8_t i = 0;

	for (i = 0; i < 4; i++) {
		byte_data = 0;
		byte_data = (data & (0xF << (4 * i))) >> (4 * i);
		cdata[3 - i] = "0123456789ABCDEF"[byte_data & 0x0F];

	}
	cdata[4] = 0;

}

void spi_link_init() {
	const uint8_t blah[2] = { 0x01, 0x03 };
	P2DIR |= BIT1;
	msp430_spi_setup(); //Setup SPI
	P2OUT &= ~BIT1; //Reset Slave Micro
	_delay_cycles(10000); //Necessary Delay
	P2OUT |= BIT1; //De Assert Reset Line
	_delay_cycles(1000000);
	spi_sends((uint8_t *) blah, 2); //Send Sync Bits

	while (!sync_complete) {

	}

}

void spi_send_packet() {
	const uint8_t blah[4] = { 0x01, 0x0A, 0x0A, 0x0B };
	spi_sends((uint8_t *) blah, 4);
	_delay_cycles(100);

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

	static char tx_buffer[2];
	static bool first_byte = true;
	static bool second_byte = 0;
	static bool third_byte = 0;
	static bool stop_byte = 0;
	uint8_t adc_data = 0;
	static uint16_t buffer_adc_data = 0;

	if ((IFG2 & UCA0RXIFG)) // IF UART DATA
	{
		tx_buffer[0] = UCA0RXBUF;

		switch (tx_buffer[0]) {

		case '1': //Reset MCU
			__disable_interrupt(); //Erata issue
			TACCR0 = 0; //Disable Timer // apparent Erata?
			_delay_cycles(1000000); //Erata issue
			clearDisplay()
			; //Erata issue
			_delay_cycles(1000); //Erata issue
			sys_reset(); //Enable WDT to cause reset
			_delay_cycles(1000000); //Erata issue
			break;

		case '2': //Display ADC Data to terminal
			test_buffer = global_adc_data;
			data_to_pc(test_buffer, g_buffer);
			uart_sends(g_buffer);
			break;

		case '3':
			uart_sends("Connected");
			break;

		case '4':
			start_data_collect = true;
			break;


		default: //Else Error
			uart_sends("error");
			break;

		} // End TX Switch

	}

	if ((IFG2 & UCB0RXIFG)) // IF SPI DATA
	{

		adc_data = UCB0RXBUF;

		if (!sync_complete) { //First Sync SPI Data
			if (adc_data == 0x04) {
				sync_complete = true;
				P1OUT |= BIT0;
			}
		} else {

			if ((adc_data == 0x06) & (first_byte)) { // SPI 4 byte packet implementation
				second_byte = true;
				first_byte = false;
			}

			else if (second_byte) {
				second_byte = false;
				third_byte = true;
			}

			else if (third_byte) {
				third_byte = false;
				stop_byte = true;
				buffer_adc_data = (adc_data << 8);

			}

			else if (stop_byte) {
				stop_byte = false;
				first_byte = true;
				buffer_adc_data |= adc_data;
				global_adc_data = buffer_adc_data;
			}
		}
	}

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void) {

	static uint16_t counts = 0; //Update screen with latest info every 50ms
	if (counts > 10) {
		counts = 0;
		update_screen();

	}

	counts++;
}

void update_screen() {
	static uint16_t previous_data = 0;
	uint16_t data = 0;
	char t_buffer[8];
	data = global_adc_data;

	if (previous_data < data) { //Check for peak data
		data_to_mw(data, t_buffer);
		first_line_p(); //Move Cursor for peak data
		Print_Screen(t_buffer); //Write Peak Data
		previous_data = data;
	}
	second_line_p(); //Move Cursor for Latest Data
	data_to_mw(data, t_buffer);
	Print_Screen(t_buffer); //Write Latest data

}

void data_to_mw(uint16_t data, char *cdata) {
	uint16_t local_data = data;
	uint16_t con_data= data;

	if(!startup){
		if(con_data>0x80) {
			local_data = local_data +14;
			local_data = local_data/40;
		}
		else if(con_data>0x14) {
			local_data = local_data - 0x16;
			local_data = local_data / 4;
		}
		else {
			local_data=0;
		}

	}

	cdata[0]= (local_data / 1000) + '0';
	local_data = (local_data % 1000);
	cdata[1] = (local_data / 100) + '0';
	local_data = (local_data % 100);
	cdata[2]= (local_data / 10) + '0';
	local_data = (local_data % 10);
	cdata[3]= (local_data) + '0';
	cdata[4] = ' ';
	cdata[5] = 'm';
	cdata[6] = 'W';
	cdata[7] = '\0';


}

void data_to_pc(uint16_t data, char *cdata) {

	uint16_t local_data = data;
	//uint16_t con_data= data;

	local_data = local_data/40;


	cdata[0]= (local_data / 1000) + '0';
	local_data = (local_data % 1000);
	cdata[1] = (local_data / 100) + '0';
	local_data = (local_data % 100);
	cdata[2]= (local_data / 10) + '0';
	local_data = (local_data % 10);
	cdata[3]= (local_data) + '0';
	cdata[4] = '\0';


}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{

	if((P2IN & BIT6)==BIT6) {
		__disable_interrupt(); //Erata issue
		TACCR0 = 0; //Disable Timer // apparent Erata?
		_delay_cycles(1000); //Erata issue
		sys_reset(); //Enable WDT to cause reset
		_delay_cycles(1000000); //Erata issue
	}

	if((P2IN & BIT7)==BIT7) {

		if(start_data_collect)
		{
			start_data_collect=false;
			TACCR0 = 0;
		}

		else
		{
			start_data_collect=true;
			TACCR0 = 10000;
		}
		_delay_cycles(500000);



	}

	P2IFG &= ~(BIT6 + BIT7);
}


