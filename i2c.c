#include <avr/io.h>
#include <avr/interrupt.h>

#define DELAY_VALUE 1000

void my_delay(unsigned int ms);
void timer_setup();
void i2c_init();
void i2c_start();
void i2c_end();
void i2c_write(unsigned char data);
void lcd_init();
void lcd_send_command(char command);
void lcd_send_data(char data);
void print_message(char *s, int row, int col);

volatile int count = 0;

ISR(TIMER0_COMPA_vect) {
	count++;
}

int main(void) {

	sei();

	// Set the onboard led to be output
	DDRB |= (1 << DDB5);

	timer_setup();


	// I2C LCD
	i2c_init();
	i2c_start();
	my_delay(100);

	// Send the address and set to write mode
	i2c_write(0x27 << 1);
	my_delay(100);

	// Init the LCD and print message
	lcd_init();

	print_message("Hello there!", 0, 0);
	my_delay(2000);

	lcd_send_command(0x01);
	my_delay(20);

	print_message("Ready to learn some", 0, 0);
	print_message("Arduino Programming", 1, 0);
	my_delay(2000);

	lcd_send_command(0x01);
	my_delay(20);

	print_message("With bare-metal C", 0, 0);
	print_message("of course!", 1, 0);

	// Turn on LED

	i2c_end();
	
	/*
	=============== MAIN LOOP ==============
	*/
	while (1) {
		PORTB ^= (1 << PORTB5);
		my_delay(DELAY_VALUE);
	}

}

void my_delay(unsigned int ms) {
	count = 0;
	while (count < ms) {
		;
	}
}

void timer_setup() {

	// Set to CTC mode
	TCCR0A |= (1 << WGM01);

	// Set pre-scalar to 64. 16 MHz / 64 = 250 kHz
	// So, T = 1/250kHz = 4 us
	TCCR0B |= (1 << CS01) | (1 << CS00);

	// 4 us * 250 = 1 ms. But count is 0, 1, 2, ..., OCR0A
	// So, should minus 1 to account for zero counting
	OCR0A = 250 - 1;

	// Enable Timer0A interrupt
	TIMSK0 |= (1 << OCIE0A);
}

void i2c_init() {
	// Set pre-scalar to 72. This makes clock for I2C 100kHz
	TWBR = 72;
	TWSR = 0x00;
}

void i2c_start() {
	// TWINT sets the job finished bit to 0. Note, writing a 1 to
	// it sets it to 0. TWSTA sets the start bit. Transmitted to
	// other devices to say, transmission about to start. TWEN
	// enables I2C.
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

	// Wait for the START bit transmission to complete
	while (!(TWCR & (1 << TWINT))) {
		;
	}
}
void i2c_end() {

	// Send STOP bit
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void i2c_write(unsigned char data) {

	// Store the data in the data register
	TWDR = data;

	// Transmit the data
	TWCR = (1 << TWINT) | (1 << TWEN);

	// Wait for transmission
	while (!(TWCR & (1 << TWINT))) {
		;
	}
}

void lcd_send_command(char command) {

	// Get upper four data bits
	char data_upper = command & 0xf0;

	// Get lower four data bits
	char data_lower = (command << 4) & 0xf0;

	// Send the upper 4 bits
	data_upper = data_upper | 0x0C; // Keep backlight on and enable on
	i2c_write(data_upper);
	data_upper = data_upper & ~(0x04); // Set enable off
	i2c_write(data_upper);

	// Send the lower 4 bits
	data_lower = data_lower | 0x0C; // Set backlight on and enable on
	i2c_write(data_lower);
	data_lower = data_lower & ~(0x04); // Set enable off
	i2c_write(data_lower);
}

void lcd_send_data(char data) {

	// Get upper four data bits
	char data_upper = data & 0xf0;

	// Get lower four data bits
	char data_lower = (data << 4) & 0xf0;

	// Send the upper 4 bits
	data_upper = data_upper | 0x0d; // Set backlight on, enable on and data bit on
	i2c_write(data_upper);
	data_upper = data_upper & ~(0x04); // Set enable off
	i2c_write(data_upper);

	// Send the lower 4 bits
	data_lower = data_lower | 0x0d; // Set backlight on, enable on and data bit on
	i2c_write(data_lower);
	data_lower = data_lower & ~(0x04); // Set enable off
	i2c_write(data_lower);
}

void print_message(char *s, int row, int col) {

	int pos;
	if (row == 0) {
		 pos = col | 0x80;
	} else {
		 pos = col | 0xC0;
	}

	// Send command to say where to start text display from
	lcd_send_command(pos);

	while (*s) {
		lcd_send_data(*s);
		s++;
	}
}

void lcd_init() {

	// Initialization sequence.
	// Wait 50 ms
	my_delay(50);

	lcd_send_command(0x30);
	my_delay(5);

	lcd_send_command(0x30);
	my_delay(1);

	lcd_send_command(0x30);
	my_delay(1);

	// Set to 4-bit mode
	lcd_send_command(0x20);
	my_delay(10);

	// 2-line display, set font to 5 by 8
	lcd_send_command(0x28);
	my_delay(10);

	// Clear the display
	lcd_send_command(0x01);
	my_delay(15);

	// Turn display on, and set cursor off with blinking off
	lcd_send_command(0x0C);

	// Cursor on and blinking
	// lcd_send_command(0x0F);

	// Cursor on and no blinking
	// lcd_send_command(0x0F);

	my_delay(15);

}
