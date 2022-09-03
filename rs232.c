#include <p18f2523.h>
#include <stdlib.h>
#include <string.h>
#include <usart.h>
#include <ctype.h>
#include <delays.h>
#include <stdio.h>
#include <math.h>


#include "rs232.h"
#include "interrupts.h"
#include "dac.h"
#include "adc.h"

#pragma udata _rs232_tx_buffer
#define RS232_TX_BUFFER_SIZE	64
unsigned char _Rs232TxBuffer[RS232_TX_BUFFER_SIZE];
unsigned int _Rs232TxBufferHead = 0;
unsigned int _Rs232TxBufferTail = 0;

#pragma udata _rx_rs232_buffer 
#define RS232_RX_BUFFER_SIZE	64
unsigned char _Rs232RxBuffer[RS232_RX_BUFFER_SIZE];
unsigned int _Rs232RxBufferHead = 0;
unsigned int _Rs232RxBufferTail = 0;

#pragma udata _rs232_command_buffer 
char szCommand[80];
int nCommandIndex = 0;

#pragma code

void _rs232_rx_handler(void)
{
	unsigned char bf_count = 0, c = 0;

	if(DataRdyUSART())	// While there is data available
	{
		c = ReadUSART();

		// First we move to our next location because we need to determine if
		// there is an overrun condition
		_Rs232RxBufferHead++;

		// if we have gone past the end of the buffer go back to the 
		// beginning of the buffer.
		if(_Rs232RxBufferHead >= RS232_RX_BUFFER_SIZE)
			_Rs232RxBufferHead = 0;

		if(_Rs232RxBufferHead >= _Rs232RxBufferTail)
			bf_count = _Rs232RxBufferHead-_Rs232RxBufferTail;
		else
			bf_count = (RS232_RX_BUFFER_SIZE-_Rs232RxBufferTail) + _Rs232RxBufferHead; 

		// Test to see if we have caught up with our tail. If we have we 
		// have a communications overrun situation. Flag the error and dump
		// the received byte
		if(_Rs232RxBufferHead == _Rs232RxBufferTail)
		{
			// Step back one again.
			_Rs232RxBufferHead--;

			// And mask	as required
			if(_Rs232RxBufferHead >= RS232_RX_BUFFER_SIZE || _Rs232RxBufferHead < 0)
				_Rs232RxBufferHead = 0;
		}
		else
		{
			// Read a byte from our NMEA data streamand write our char 
			// to our buffer, we are all done
			_Rs232RxBuffer[_Rs232RxBufferHead] = c;
		}
	}
}

void _rs232_tx_handler(void)
{
	if(PIE1bits.TXIE)
	{
		if(_Rs232TxBufferHead == _Rs232TxBufferTail)
		{
			// There is no data to send or the remote devide is not ready, disable TX interrupts
			PIE1bits.TXIE = 0;
		}
		else
		{
			// Move to the next location
			_Rs232TxBufferTail++;

			if(_Rs232TxBufferTail >= RS232_TX_BUFFER_SIZE)
				_Rs232TxBufferTail = 0;

			// Write our byte
			while(BusyUSART());

			WriteUSART(_Rs232TxBuffer[_Rs232TxBufferTail]);
		}
	}
}

unsigned char IsTxBufferFull(void)
{
	unsigned char t = _Rs232TxBufferHead+1;

	if(t >= RS232_TX_BUFFER_SIZE)
		t = 0;

	if(t == _Rs232TxBufferTail)
		return 1;

	return 0;
}

void SendByte(unsigned char c)
{
	unsigned char t = 0;

	t = _Rs232TxBufferHead+1;
	if(t >= RS232_TX_BUFFER_SIZE)
		t = 0;

	if(t != _Rs232TxBufferTail)
	{
		_Rs232TxBufferHead = t;

		_Rs232TxBuffer[_Rs232TxBufferHead] = c;
	}

	// Start transmission
	PIE1bits.TXIE = 1;
}

unsigned char IsRxDataAvail(void)
{
	unsigned char ct = 0;

	if(_Rs232RxBufferHead >= _Rs232RxBufferTail)
		ct = _Rs232RxBufferHead-_Rs232RxBufferTail;
	else
		ct = (RS232_RX_BUFFER_SIZE-_Rs232RxBufferTail) + _Rs232RxBufferHead; 

	if(ct != 0)
		return 1;

	return 0;
}

void FlushRxBuffer(void)
{
	_Rs232RxBufferHead = 0;
	_Rs232RxBufferTail = 0;
}

unsigned char ReadByte(void)
{
	_Rs232RxBufferTail++;

	if(_Rs232RxBufferTail >= RS232_RX_BUFFER_SIZE)
		_Rs232RxBufferTail = 0;

	return _Rs232RxBuffer[_Rs232RxBufferTail];
}

void rs232_putrs(const rom char* str)
{
	while(*str)
	{
		SendByte(*str);
		str++;
	}
}

void rs232_puts(char* str)
{
	while(*str)
	{
		SendByte(*str);
		str++;
	}
}

void rs232_gets(char* str)
{
	while(IsRxDataAvail())
	{
		(*str) = ReadByte();
		str++;
	}
	(*str) = 0;
}

int rs232_getl(char* buff)
{
	char* ptr = buff;
	int ct = 0;

	while(IsRxDataAvail())
	{
		(*ptr) = ReadByte();

                // Terminal echo
                //SendByte(*ptr);
                
		ct++;

		if( (*ptr) == '\r' )
		{
                    // Terminal echo
                    //SendByte('\n');

                    (*ptr) = 0;
                    return -1;
		}	
		ptr++;
	}
	(*ptr) = 0;

	return ct;
}

void rs232_init(void)
{
	_Rs232TxBufferHead = 0;
	_Rs232TxBufferTail = 0;
	_Rs232RxBufferHead = 0;
	_Rs232RxBufferTail = 0;

	memset(szCommand, 0, sizeof(szCommand));

        // Enable the serial serial port
        RCSTAbits.SPEN = 1;


	// Open USART 1 for RX/TX for command and control
	OpenUSART(USART_TX_INT_ON &             // Needed for TX interrupt mode
			   USART_RX_INT_ON &	// Needed for RX interrupt mode
			   USART_ASYNCH_MODE &	// Standard Async
			   USART_EIGHT_BIT &	// 8-bit data
			   USART_CONT_RX &		// Continuous receive
			   USART_BRGH_LOW, 12); // 9600 baud @ 8Mhz clock   //[8000000/(9600*16)] - 1 = 12
}

unsigned char _do_error(unsigned char e)
{
	sprintf(szCommand, "ERROR: %d ", e);
	rs232_puts(szCommand);

	return 0;	
}

int dac_V = 0;
int dac_I = 0;

unsigned char _do_output_on(char* szVal)
{

    LATBbits.LATB5 = 0;

    write_dac_V(dac_V);
    write_dac_I(dac_I);

    return 1;
}

unsigned char _do_output_off(char* szVal)
{
    write_dac_V(0);
    write_dac_I(0);

    LATBbits.LATB5 = 1;

    return 1;
}

unsigned char _do_set_volts(char* szVal)
{
    dac_V = atoi(szVal);

    if(!PORTBbits.RB5)
        write_dac_V(dac_V);

    return 1;
}

unsigned char _do_set_current(char* szVal)
{
    dac_I = atoi(szVal);

    if(!PORTBbits.RB5)
        write_dac_I(dac_I);

    return 1;
}


int abs(int Value)
{      if (Value < 0)
            return(-Value);
    else
            return(Value);
}

/*******************************************************************
* FUNCTION: ftoa
* AUTHOR = Michael L Anderson, PhD
* COMPANY = Black Oak Engineering
* URL = www.blackoakeng.com
* EMAIL = office @ blackoakeng.com < remove spaces
* COMPILER	= MPLAB C18 C comp v3.35
* DATE	 = 10 Feb 10
*
* PARAMETERS: float f
*
* DESCRIPTION: Converts a float to fixed point string. This is obviously
* quite limited but it works well for many embedded situations and it is
* very compact. Note - C18 compiler printf() fcns do not support
* floating point args; this is one simple way to fill the deficit.
*
* RETURNS: C string pointer
*

*******************************************************************/
#include <string.h>
#include <stdlib.h>

ram char bfr1[21], bfr2[11];

char *ftoa(float f)
{
    unsigned char ctr = 0;
    unsigned u;

    if (f < 0.0) { bfr1[ctr++] = '-'; f *= -1; }
        bfr1[ctr] = 0;	 // Terminate C string

    if (f > 0xFFFF)
    {
        strcpypgm2ram(bfr1, (rom char *) "Overrange\r\n");
        return(bfr1);
    }

    u = f;
    itoa(u, bfr2);
    strcat(bfr1, bfr2);
    ctr = strlen(bfr1);
    bfr1[ctr++] = '.';
    bfr1[ctr] = 0;

    f -= u;
    u = (10 * f);	 // Four sig figs after fixed dec pt

//    if (u < 9) strcpypgm2ram(bfr2, (rom char *) "000");	 // Pack the leading zeros. This actually
//    else if (u < 99) strcpypgm2ram(bfr2, (rom char *) "00");	// compiles tighter than a loop.
//    else if (u < 999) strcpypgm2ram(bfr2, (rom char *) "0");
//    else bfr2[0] = 0;
    bfr2[0] = 0;

    strcat(bfr1, bfr2);
    itoa(u, bfr2);
    strcat(bfr1, bfr2);


    return(bfr1);
}


unsigned char _do_status(char* szVal)
{
    float degrees = 0.0;
    char on = !LATBbits.LATB5;
    char cv = PORTAbits.RA6;
    char cc = (PORTA & 0x80) ? 1 : 0;

    int line_freq = get_line_frequency();

    int in_reg = 1;
    if(cv == 0 && cc == 0)
        in_reg = 0;

    // Clear down our last ADC values
    T_mon = V_mon = I_mon = PR_mon = 0;

    // REad our ADC channels
    adc_read();

    // Adjust for our DC offset. zero degress sits at 1196. This should be
    // a calibration setting.
    degrees = (T_mon - 1196);

    // We are getting about 5.5 steps per degree
    degrees /= 5.5;

    sprintf(szCommand, "Vs=%d,Cs=%d,Va=%u,Ca=%u,Pr=%d,Tm=%s,cc=%d,cv=%d,Lf=%d,Op=%d,Rg=%d\r\n",
                        dac_V, dac_I, V_mon, I_mon, PR_mon, ftoa(degrees), cc, cv, line_freq, on, in_reg);

    rs232_puts("Hello");

    rs232_puts(szCommand);

    return 1;
}


////////////////////////////////////////////////////////////////
// Command Format
//
// Commands:
//   V xxxx - set the volts
//   C xxxx - set the current
//   N      - turns the output oN
//   F      - turns the output oFf
//   S      - returns the status of the regulator
//
// Status Format:
//   Vs=xxxx,Cs=xxxx,Va=xxxx,Ca=xxxx,Pr=xxxx,cc=x,cv=x,Lf:xx
//
////////////////////////////////////////////////////////////////

void _do_command(char* szCommand)
{
	char ret = 0;

	switch(szCommand[0])
	{
	case 'C':
	case 'c':
		ret = _do_set_current(&szCommand[1]);
		break;
	case 'V':
	case 'v':
		ret = _do_set_volts(&szCommand[1]);
		break;
	case 'N':
	case 'n':
		ret = _do_output_on(&szCommand[1]);
		break;
	case 'F':
	case 'f':
		ret = _do_output_off(&szCommand[1]);
		break;
        default:
            break;
	}
	
        _do_status(&szCommand[1]);
//       	sprintf(szCommand, "\r\n", ret);
//        rs232_puts(szCommand);
}

void rs232_tasks(void)
{
	char* p;

	// Read our command text off the receive buffer
	int nCount = rs232_getl(&szCommand[nCommandIndex]);

	if(nCount == -1)
	{
		nCommandIndex = 0;
		
		p = strrchr(szCommand, ':');
		if(p)
			p++;
		else
			p = szCommand;

		_do_command(p);

		memset(szCommand, 0, sizeof(szCommand));
	}
	else
	if(nCount)
	{
		nCommandIndex += nCount;
	}
}
