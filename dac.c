#include <p18f2523.h>
#include <spi.h>

#define DAC_CS  LATCbits.LATC2

#define SET_TIMER0(t) TMR0H = (t>>8); TMR0L = (t&0xff)
#define SET_TIMER1(t) TMR1H = (t>>8); TMR1L = (t&0xff)

unsigned char V_dac_val_1[4];
unsigned char V_dac_val_2[4];
unsigned char I_dac_val_1[4];
unsigned char I_dac_val_2[4];
char dac_dither_pos = 0;

unsigned int _timer1_val;
unsigned int _current_tick = 0;

unsigned int _line_frequency = 0;

unsigned int get_line_frequency(void)
{
    return _line_frequency;
}

void write_dac_V(int val)
{
    unsigned int dac_val = 0, dac_val2 = 0;
    char dac_lsb = 0, x = 4;

    dac_lsb = val&0x3;
    dac_val = (val >> 2);
    dac_val2 = dac_val + 1;

    // Stop the timer interrupts from updating the DAC
    PIE1bits.TMR1IE = 1;		// Enable interrupts on this timer (terminal count of ZERO)
    PIR1bits.TMR1IF = 0;		// Clear interrupt flag (just in case)

    do {
        x--;

        if(dac_lsb >= x)
        {
            V_dac_val_1[x] = 0x10 + ((dac_val2 & 0x0f00) >> 8);
            V_dac_val_2[x] = dac_val2 & 0xff;
        }
        else
        {
            V_dac_val_1[x] = 0x10 + ((dac_val & 0x0f00) >> 8);
            V_dac_val_2[x] = dac_val & 0xff;
        }
    } while(x);

    dac_dither_pos = 0;

    PIE1bits.TMR1IE = 1;		// Enable interrupts on this timer (terminal count of ZERO)

    SET_TIMER1(_timer1_val);
}

void write_dac_I(int val)
{
    unsigned int dac_val = 0, dac_val2 = 0;
    char dac_lsb = 0, x = 4;

    dac_lsb = val&0x3;
    dac_val = (val >> 2);
    dac_val2 = dac_val + 1;

    // Stop the timer interrupts from updating the DAC
    PIE1bits.TMR1IE = 1;		// Enable interrupts on this timer (terminal count of ZERO)
    PIR1bits.TMR1IF = 0;		// Clear interrupt flag (just in case)

    do {
        x--;

        if(dac_lsb >= x)
        {
            I_dac_val_1[x] = 0x90 + ((dac_val2 & 0x0f00) >> 8);
            I_dac_val_2[x] = dac_val2 & 0xff;
        }
        else
        {
            I_dac_val_1[x] = 0x90 + ((dac_val & 0x0f00) >> 8);
            I_dac_val_2[x] = dac_val & 0xff;
        }
    } while(x);

    dac_dither_pos = 0;

    PIE1bits.TMR1IE = 1;		// Enable interrupts on this timer (terminal count of ZERO)

    SET_TIMER1(_timer1_val);
}

void spi_init(void)
{
    // Emsure that our DAC chip select is high
    DAC_CS = 1;

    // Open our SPI pheripheral
    OpenSPI(SPI_FOSC_4, MODE_00, SMPEND);

    write_dac_V(0);
    write_dac_I(0);

    _timer1_val = 63780;        // We are generating 1000 interrupts per second
                                // on the high speed timer which means our dither
                                // rate is 250 cycles per second with tro bit
                                // resolution.
                                // This is also used to calculate the 50/60Hz line
                                // frequency so it needs to be as close to 1Khz as
                                // possible

    // Enable Timer 1, 16bit mode, prescale @ 1:1, 100ns per tick-the highest resolution we can get running
    // the MCU @ 40MHz (HS4)
    T1CON = 0x85;			// 16bit, 1:1 Prescale, No Osc, No TSync, Fosc/4 Clk Src, Enabled (10000101b)
    PIR1bits.TMR1IF = 0;		// Clear interrupt flag (just in case)
    IPR1bits.TMR1IP = 1;		// Make interrupts high priority
    PIE1bits.TMR1IE = 1;		// Enable interrupts on this timer (terminal count of ZERO)

    SET_TIMER1(_timer1_val);
}

void high_pri_interrupt_handler(void);

#pragma code _hi_pri_interrupt = 0x8
void _hi_pri_interrupt(void)
{
	_asm goto high_pri_interrupt_handler _endasm
}
#pragma code

#pragma interrupt high_pri_interrupt_handler
void high_pri_interrupt_handler(void)
{
    unsigned char x;

    if(PIR1bits.TMR1IF)
    {
        // Re-start the timer with teh current timer value
        SET_TIMER1(_timer1_val);

        // We have a high speed timer (TIMER1) interrupt, reset the flag ready for the next one
        PIR1bits.TMR1IF = 0;

        LATCbits.LATC0 = 0;

        DAC_CS = 0;
        WriteSPI(V_dac_val_1[dac_dither_pos]);
        WriteSPI(V_dac_val_2[dac_dither_pos]);
        DAC_CS = 1;

        DAC_CS = 0;
        WriteSPI(I_dac_val_1[dac_dither_pos]);
        WriteSPI(I_dac_val_2[dac_dither_pos]);
        DAC_CS = 1;

        dac_dither_pos++;
        dac_dither_pos &= 0x3;

        LATCbits.LATC0 = 1;

        _current_tick++;
    }
    else
    if(INTCONbits.INT0F)
    {
	// Clear the falg, ready for next one
	INTCONbits.INT0F = 0;

        // _current_tick is incremented 10 times every second

        _line_frequency = _current_tick * 5;
                
        _current_tick = 0;
    }
}