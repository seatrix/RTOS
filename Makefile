all:
	avr-gcc -mmcu=atmega2560 main.c -c -o main.o -O3
	avr-gcc -mmcu=atmega2560 init.c -c -o init.o -O3
	avr-gcc -mmcu=atmega2560 main.o init.o -o main.elf -O3
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	sudo avrdude -c stk600 -p atmega2560 -P usb -v -v -U flash:w:main.hex

clean:
	rm -f *.o *~ main.elf main.hex
