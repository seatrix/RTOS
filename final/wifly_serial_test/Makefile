################################################################################
# Makefile for CPE439 final project Wifly test module.                         #
# @author Matt Zimmerer                                                        #
################################################################################

CC=avr-gcc
OC=avr-objcopy
SZ=avr-size
PR=avrdude

TARGET=lcdtest
SRC=main.c serial.c
F_CPU=8000000UL

CFLAGS=-mmcu=atmega2560 -Wall -std=gnu99 -O3 -DF_CPU=$(F_CPU)
LFLAGS=-mmcu=atmega2560 -DF_CPU=$(F_CPU)
OFLAGS=-j .text -j .data
PRFLAGS=-c stk600 -p ATMEGA2560 -P usb -b 115200

OBJS=$(SRC:.c=.o)

all: $(TARGET).hex

$(TARGET).hex: $(OBJS)
	$(CC) $^ -o $(TARGET).elf $(LFLAGS)
	$(OC) -O ihex $(TARGET).elf $(TARGET).hex $(OFLAGS)
	$(SZ) $(OBJS) $(TARGET).elf $(TARGET).hex

install: $(TARGET).hex
	sudo $(PR) $(PRFLAGS) -U flash:w:$(TARGET).hex

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	rm -f *.elf *.hex *.o *~
