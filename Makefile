all: hello.vvp my_task.vpi

my_task.o: my_task.c
	gcc -Wall -g -c -fno-diagnostics-show-caret -fpic my_task.c  -I/usr/include/iverilog -IC:\MinGW\msys\1.0\local\include\iverilog 

my_gui.o: my_gui.c
	gcc -Wall -g -c -fno-diagnostics-show-caret -fpic my_gui.c  `pkg-config --cflags gtk+-3.0`

my_task.vpi: my_task.o my_gui.o
	gcc -Wall -g -shared -o my_task.vpi my_task.o my_gui.o -LC:\MinGW\msys\1.0\local\lib -lvpi -lrt `pkg-config --libs gtk+-3.0`

hello.vvp:	hello.v
	iverilog -o hello.vvp hello.v

run: hello.vvp my_task.vpi
	vvp -M. -mmy_task hello.vvp

debug: hello.vvp my_task.vpi
	gdb --args vvp -M. -mmy_task hello.vvp

clean:
	rm -rf *.o
	rm -rf *.vpi
	rm -rf *.vvp


gtktest:	gtktest.c
	gcc -o gtktest gtktest.c -Wall -fno-diagnostics-show-caret `pkg-config --cflags --libs gtk+-3.0`
