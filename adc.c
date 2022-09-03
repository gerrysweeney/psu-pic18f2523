#include <p18f2523.h>
#include <adc.h>
#include <delays.h>

unsigned int V_mon = 0;
unsigned int I_mon = 0;
unsigned int PR_mon = 0;
unsigned int T_mon = 0;

void adc_init(void)
{
    // Turn on the ADC peripheral
    ADCON1bits.VCFG0 = 0;   // Vss
    ADCON1bits.VCFG1 = 0;   // Vdd
    ADCON1bits.PCFG = 0x8;  // AD 0-7
    ADCON2bits.ADFM = 1;
    ADCON2bits.ACQT = 0x2;  // Tad=2
    ADCON2bits.ADCS = 0;

    ADCON0bits.ADON = 1;
}

void adc_read(void)
{
    /////////////////////////////////////////////////////////////////////////
    // Measure our current
    ADCON0bits.CHS = 0;                 // We want to sample Channel AN0
    ADCON0bits.GO = 1;                  // Start the conversion now
    while(ADCON0bits.DONE == 1)         // And wait for it to complete
        Delay1KTCYx(1);
    V_mon = (ADRESH * 256) + ADRESL;    // Save the result
    /////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////
    // Measure our current
    ADCON0bits.CHS = 1;                 // We want to sample Channel AN1
    ADCON0bits.GO = 1;                  // Start the conversion now
    while(ADCON0bits.DONE == 1)         // And wait for it to complete
        Delay1KTCYx(1);
    I_mon = (ADRESH * 256) + ADRESL;   // Save the result
    /////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    // Measure our temperature
    ADCON0bits.CHS = 4;                 // We want to sample Channel AN4
    ADCON0bits.GO = 1;                  // Start the conversion now
    while(ADCON0bits.DONE == 1)         // And wait for it to complete
        Delay1KTCYx(1);
    T_mon = (ADRESH * 256) + ADRESL;   // Save the result
    /////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////
    // Measure our pre-regulator voltage
//    ADCON0bits.CHS = 11;                // We want to sample Channel AN11
//    ADCON0bits.GO = 1;                  // Start the conversion now
//    while(ADCON0bits.DONE == 1)         // And wait for it to complete
//        Delay1KTCYx(1);
//    PR_mon = (ADRESH * 256) + ADRESL;   // Save the result
    /////////////////////////////////////////////////////////////////////////




}
