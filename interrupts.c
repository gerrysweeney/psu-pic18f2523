#include <p18f2523.h>

#include <string.h>
#include <delays.h>

#include "interrupts.h"
#include "rs232.h"
#include "dac.h"


#pragma code
void interrupt_init(void)
{
    // Enable interrupt priority
    RCONbits.IPEN = 1;

    // Make RX interrupts low priority for USART
    IPR1bits.RCIP = 0;

    // Make TX interrupts low priority for USART
    IPR1bits.TXIP = 0;

    // Enable all high priority interrupts
    INTCONbits.GIEH = 1;

    // Enable all low priority interrupts
    INTCONbits.GIEL = 1;

    // Enable falling edge interrupts on RB0. RBO is fed with a full-wave rectified signal.  In order
    // to account for interrupt latentcy we trigger on the falling edge of the half cycle.  The latency
    // time brings us pretty much into the correct point of the start of the next half-cycle.
    INTCON2bits.INTEDG0 = 0;	// Falling Edge
    INTCONbits.INT0E = 1;
}

//void high_pri_interrupt_handler(void);
void low_pri_interrupt_handler(void);

/*#pragma code _hi_pri_interrupt = 0x8
void _hi_pri_interrupt(void)
{
	_asm goto high_pri_interrupt_handler _endasm
}
#pragma code
*/

#pragma code _lo_pri_interrupt = 0x18
void _lo_pri_interrupt()
{
	_asm goto low_pri_interrupt_handler _endasm
}
#pragma code

/*
#pragma interrupt high_pri_interrupt_handler
void high_pri_interrupt_handler(void)
{
	if(PIR1bits.TMR1IF)
            _dac_hs_interrupt_handler();

//	if(INTCONbits.INT0F)
//	{
//		_phase_controller_zero_cross_interrupt_handler();
//	}
}
*/
#pragma interruptlow low_pri_interrupt_handler
void low_pri_interrupt_handler(void)
{
	// Process any RS232 input
	if(PIR1bits.RCIF)
		_rs232_rx_handler();

	if(PIR1bits.TXIF)
		_rs232_tx_handler();
}

