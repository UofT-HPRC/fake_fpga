# From https://stackoverflow.com/questions/714100/os-detecting-makefile


ifeq ($(OS),Windows_NT)
	CCFLAGS = -IC:\MinGW\msys\1.0\local\include\iverilog -LC:\MinGW\msys\1.0\local\lib -lvpi -lws2_32
else 
	CCFLAGS = -I/usr/include/iverilog -lvpi
endif 

all: hello.vvp my_task.vpi

my_task.o: my_task.c
	gcc -Wall -g -c -fno-diagnostics-show-caret -fpic my_task.c $(CCFLAGS)

my_task.vpi: my_task.o
	gcc -Wall -g -shared -o my_task.vpi my_task.o $(CCFLAGS)

hello.vvp:	hello.v
	iverilog -o hello.vvp hello.v

tb.vvp:	tb.v hello.v
	iverilog -o tb.vvp tb.v

run: tb.vvp my_task.vpi
	vvp -M. -mmy_task tb.vvp

debug: hello.vvp my_task.vpi
	gdb --args vvp -M. -mmy_task hello.vvp

clean:
	rm -rf *.o
	rm -rf *.vpi
	rm -rf *.vvp
