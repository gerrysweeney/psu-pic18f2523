#ifndef _RS232_HEADER
#define _RS232_HEADER

// Called at start-up to initialise the Comms and data structures needed
// to process RS232. The data is expected on USART 1 at 9600-N-8-1
void rs232_init(void);

// This is called from the main loop to allow any rs232 realted user
// taskes to be executed.  This is designed to work in a co-operative
// multi-tasking design and should be called as often as possible
void rs232_tasks(void);

// Write text from ROM
void rs232_putrs(const rom char* str);

// Write text from RAM
void rs232_puts(char* str);

// Get string into ram
void rs232_gets(char* str);

// Get string terminated by a \n. If the read is complete, it will return -1, if there
// is no data available, it will return 0, if the read is incomplete it will return
// the number of bytes read
int rs232_getl(char* buff);

// These are the interrupt handlers for the GSM data. These should be called
// from the high-priority interrupt vecotr depending on what the interrupt 
// source was
extern void _rs232_rx_handler(void);
extern void _rs232_tx_handler(void);

#endif //_RS232_HEADER
