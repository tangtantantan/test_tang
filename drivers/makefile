all : button.o  led.o
	arm-none-eabi-ld -r $^  -o drivers.o
	
%.o : %.c
	@arm-none-eabi-gcc ${header_file} -c $^
	
%.o : %.S
	arm-none-eabi-gcc ${header_file} -c $^

clean:
	-rm -f *.o *.bak		





