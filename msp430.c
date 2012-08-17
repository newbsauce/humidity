#include "msp430.h"

//MSP430.h library written by Nathan Zimmerman to ease use of msp430 coding
#define SW1		BIT6
#define SW2		BIT7



void delay_ms(uint16_t ms)
{
uint16_t i = 0;

for(i=0; i<ms; i++)
{
_delay_cycles(982);
}


}

void msp430_clk_setup()
{
	WDTCTL = WDTPW + WDTHOLD; // disable WDT
	BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
	DCOCTL = CALDCO_1MHZ;
}


void msp430_spi_setup()
{
	P1SEL |= BIT5 + BIT6 + BIT7;
	P1SEL2 |= BIT5 + BIT6 + BIT7;

	UCB0CTL0 |= UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master

	//UCMSB Most significant bit first
	//UCCKPL The inactive state is high.
	//UCMST Master Mode
	//UCSYNC Synchronous mode

	UCB0CTL1 |= UCSSEL_2; // SMCLK
	UCB0BR0 |= 0x0A; // 1MHZ / 10 = 100khz SCLK
	UCB0BR1 = 0; // 1MHZ / 10 = 100khz SCLK
	UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
	IE2 |= UCB0RXIE; //Enable interrupt for USCI B
	__enable_interrupt(); //Guess

}

void spi_sends(uint8_t * texts, uint8_t length,  uint8_t chip_select)
{
uint8_t i=0;

//P1OUT &= ~chip_select;
P2OUT &= ~chip_select;


for(i = 0; i<length; i++)
	{
	UCB0TXBUF = texts[i]; // write INT to TX buffer
	while (!(IFG2 & UCB0TXIFG));
	}

while ((UCB0STAT & UCBUSY));

P1OUT |= chip_select;

}


void mps430_uart_setup()
{

	P1SEL = BIT1 + BIT2; // Set P1.1 to RXD and P1.2 to TXD
	P1SEL2 = BIT1 + BIT2; //

	UCA0CTL1 |= UCSSEL_2; // Have USCI use SMCLK AKA 1MHz main CLK
	UCA0BR0 = 104; // Baud: 9600, N= CLK/Baud, N= 10^6 / 9600
	UCA0BR1 = 0; // Set upper half of baud select to 0
	UCA0MCTL = UCBRS_1; // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST; // Start USCI
	IE2 |= UCA0RXIE; //Enable interrupt for USCI A
	P1OUT = BIT1 + BIT2; //Debug LEDs
	__enable_interrupt(); //Guess

}

void uart_sends(char *texts)
{
	unsigned int i = 0; //Define end of string loop int
	char *message; // message variable
	unsigned int message_num; // define ascii int version variable
	message = texts; // move tx_message into message

		while (1) {

			message_num = (int) message[i]; //Cast string char into a int variable

			UCA0TXBUF = message_num; // write INT to TX buffer

			i++; // increase string index

			__delay_cycles(2500); //transmission delay

			if (i >40) //prevent infinite transmit
					{
				//P1OUT |= ERROR;
				break;
			}

			if (message[i] == 0) // If end of input string is reached, break loop.
					{
				__delay_cycles(500);
				UCA0TXBUF = 0;
				__delay_cycles(2500);
				break;
			}

		} // End TX Main While Loop

}

void msp430_port_setup()
{

	P1DIR |= wRESET + wCS;
	P1OUT |= wRESET + wCS;
	P2DIR |= wPWND;
	P2OUT |= wPWND;
}

void switch_setup()
{
	P2SEL &=~(SW1 + SW2);
	P2DIR &=~(SW1 + SW2);
	P2REN |=(SW1 + SW2);
	P2OUT |=(SW1 + SW2);

}

void sys_reset()
{
	WDTCTL = WDT_MRST_32 + ~WDTHOLD;
	_delay_cycles(120000);
}

void msp430_set_timer_A(uint16_t counts)
{
	TACCTL0  = CCIE;
	TACCR0 = counts;
	TACTL  = TASSEL_2 + MC_1;

	//TASSEL_2 --> Select SMCLK as primary clk
	//MC_1 --> up mode, count to value set
	//timer interrupt enable

	__enable_interrupt(); //Guess

}

