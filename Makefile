PORT = /dev/ttyACM0
BAUD = 115200
PROGRAMMER = arduino
DEVICE = atmega328p
# -Os flag is optimize for size
COMPILE = avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=$(DEVICE)

.PHONY: all compile upload clean

all: compile upload clean

compile:
	$(COMPILE) *.c -o blink.elf
	avr-objcopy -O ihex blink.elf blink.hex
	avr-size --format=avr --mcu=$(DEVICE) blink.elf

upload:
	avrdude -v -p $(DEVICE) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U flash:w:blink.hex:i

clean:
	rm blink.elf
	rm blink.hex
