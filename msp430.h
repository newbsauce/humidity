/*
 * msp430.h
 *
 *  Created on: Jul 4, 2012
 *      Author: Nate
 */
#include "msp430g2553.h"
#include "stdint.h"

#ifndef MSP430_H_
#define MSP430_H_

//defines for MSP430

#define wRESET					BIT0 // Port 1
#define uTX						BIT1
#define uRX						BIT2
#define wINT					BIT3
#define wCS						BIT4
#define wSCLK					BIT5
#define wMISO					BIT6
#define wMOSI					BIT7

#define wPWND					BIT0 // Port 2

void delay_ms(uint16_t ms);
void msp430_clk_setup();
void msp430_spi_setup();
void spi_sends(uint8_t *texts, uint8_t length,  uint8_t chip_select);
void mps430_uart_setup();
void uart_sends(char *texts);
void msp430_port_setup();
void switch_setup();
void sys_reset();
void msp430_set_timer_A(uint16_t counts);









#endif /* MSP430_H_ */
