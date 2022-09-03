void spi_init(void);
void write_dac_V(int val);
void write_dac_I(int val);


// High speed interrupt used for DAC dithering
void _dac_hs_interrupt_handler(void);

// Get the currently detected line frequency
unsigned int get_line_frequency(void);
