all: hello.vvp my_task.vpi

my_task.o: my_task.c
	gcc -Wall -g -c -fno-diagnostics-show-caret -fpic my_task.c -I/usr/include/iverilog -IC:\MinGW\msys\1.0\local\include\iverilog 
	
my_task.vpi: my_task.o
	gcc -Wall -g -shared -o my_task.vpi my_task.o -LC:\MinGW\msys\1.0\local\lib -lvpi

hello.vvp:	hello.v
	iverilog -o hello.vvp hello.v

run: hello.vvp my_task.vpi
	vvp -M. -mmy_task hello.vvp
	
clean:
	rm -rf *.o
	rm -rf *.vpi
	rm -rf *.vvp
