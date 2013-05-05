###############################################################################
# Filename: Makefile
# Revision log:
###############################################################################

PROJNAME=main

CFLAGS=-funsigned-char -funsigned-bitfields
CFLAGS+=-fpack-struct -fshort-enums -Wall -c -std=gnu99
CFLAGS+=-I../../Source/portable 
CFLAGS+=-I../../Source/include 
CFLAGS+=-I../../Source/MemMang  
CFLAGS+=-mmcu=atmega2560

LDFLAGS=--Map=$(PROJNAME).map -lm 
LDFLAGS+=-L../../Source/include 
LDFLAGS+=-L../../Source/portable 
LDFLAGS+=-L../../Source/MemMang 
LDFLAGS+=-mmcu=atmega2560

HEXFLAGS=-O ihex -R .eeprom -R .fuse -R .lock -R .signature

C_SRCS =  \
../$(PROJNAME).c \
../croutine.c \
../heap_1.c \
../list.c \
../port.c \
../queue.c \
../tasks.c \
../timers.c \
../spi_sseg.c

OBJS =  \
$(PROJNAME).o \
croutine.o \
heap_1.o \
list.o \
port.o \
queue.o \
tasks.o \
timers.o \
spi_sseg.o \


./%.o: .././%.c
	avr-gcc $(CFLAGS) -o$@ $< 

all: $(PROJNAME).hex

$(PROJNAME).hex: $(OBJS)
	avr-gcc -o$(PROJNAME).elf $(OBJS) $(LDFLAGS)
	avr-objcopy $(HEXFLAGS) $(PROJNAME).elf $(PROJNAME).hex
	avr-size $(PROJNAME).elf

install: $(PROJNAME).hex
	sudo avrdude -c stk600 -p atmega2560 -P usb -v -v -U flash:w:$(PROJNAME).hex

clean:
	rm -rf *~ ./*o ./*d *.elf *.a *.hex *.lss *.eep *.map *.srec
