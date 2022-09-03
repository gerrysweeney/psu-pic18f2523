// PIC18F2523 Configuration Bit Settings
#include <p18f2523.h>

// CONFIG1H
#pragma config OSC = INTIO67    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
//#pragma config OSC = HSPLL    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)

#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

#include <stdio.h>
#include <stdlib.h>
#include <delays.h>
#include <spi.h>


#include "interrupts.h"
#include "rs232.h"
#include "dac.h"
#include "adc.h"

void main(void)
{
    unsigned int x = 0;

    // Initial internal clock configuration
    OSCCONbits.IRCF = 0x7;  // Run at 8Mhz (internal oscillator block)
    OSCCONbits.SCS = 0x2;   // Use internal oscillator block

    // Enable the PLL, we are going to clock at 32Mhz
    OSCTUNEbits.PLLEN = 1;

    TRISA = 0xd0;		// RA7/RA7 as inputs for CV and CC state monitor
    TRISAbits.RA0 = 1;          // I_MON (analog input) AN0
    TRISAbits.RA1 = 1;          // V_MON (analog input) AN1
    TRISAbits.RA5 = 1;          // T_MON (analog input) AN4

    TRISB = 0x17;
    //TRISBbits.RB0 = 1;          // AC Sense (zero cross 50/60Hz)
    //TRISBbits.RB1 = 1;          // CV_MODE (on for CV, off for no voltage regulation)
    //TRISBbits.RB2 = 1;          // CC_MODE (on for CC, off for no current regulation)
    //TRISBbits.RB3 = 0;          // PRE_REG (drive signal for pre-regulator switch)
    //TRISBbits.RB4 = 1;          // PRV_MON (analog input)
    //TRISBbits.RB5 = 0;          // OUT_OFF (drive signal, HIGH to turn output off)

    //
    INTCON2bits.RBPU = 0;         // Enable (set to 0) internal pullups on PORTB


    TRISC = 0x3;                    // PORTC all outputs
    //TRISCbits.RC0 = 1;            // RC0 input CC mode
    //TRISCbits.RC1 = 1;            // RC1 input CV mode

    // Status LED off
    LATCbits.LATC0 = 0;

    // Enable system interrupts
    interrupt_init();

    // Initialize the RS232 data strcutures and comms resources
    rs232_init();

    // Initialize the SPI bus
    spi_init();

    // Initialise the ADC
    adc_init();

    // Do our main loop
    while(1)
    {
        // Allow the RS232 interface to do work it needs to
        rs232_tasks();




    }
}