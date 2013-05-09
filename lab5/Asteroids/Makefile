###############################################################################
# Filename: Makefile
# Revision log:
###############################################################################

PROJNAME=asteroids

CFLAGS=-Wall -std=gnu99 -O3
CFLAGS+=-I../Source/portable 
CFLAGS+=-I../Source/include 
CFLAGS+=-I../Source/MemMang  
CFLAGS+=-mmcu=atmega2560

LDFLAGS=--Map=$(PROJNAME).map -lm 
LDFLAGS+=-L../Source/include 
LDFLAGS+=-L../Source/portable 
LDFLAGS+=-L../Source/MemMang 
LDFLAGS+=-mmcu=atmega2560

HEXFLAGS=-O ihex -R .eeprom -R .fuse -R .lock -R .signature

C_SRCS =  \
   $(PROJNAME).c \
   croutine.c \
   heap_2.c \
   list.c \
   port.c \
   queue.c \
   tasks.c \
   timers.c \
   graphics.c \
   usart.c

OBJS=$(C_SRCS:.c=.o)

./%.o: %.c
	avr-gcc $(CFLAGS) -c -o $@ $< 

all: $(PROJNAME).hex

$(PROJNAME).hex: $(OBJS)
	avr-gcc -o$(PROJNAME).elf $(OBJS) $(LDFLAGS)
	avr-objcopy $(HEXFLAGS) $(PROJNAME).elf $(PROJNAME).hex
	avr-size $(PROJNAME).elf

install: $(PROJNAME).hex
	sudo avrdude -c stk600 -p atmega2560 -P usb -v -v -U flash:w:$(PROJNAME).hex

clean:
	rm -rf *~ ./*o ./*d *.elf *.a *.hex *.lss *.eep *.map *.srec
