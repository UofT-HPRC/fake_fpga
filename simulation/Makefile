# From https://stackoverflow.com/questions/714100/os-detecting-makefile

CFLAGS = -c -Wall -g -fno-diagnostics-show-caret -fpic -Iinclude
LDFLAGS = -g -shared -Llib -lmtipli

ifeq ($(OS),Windows_NT)
	LDFLAGS += -lws2_32
endif 

all: fakefpga.vpi

my_task.o: my_task.c
	gcc my_task.c $(CFLAGS)

fakefpga.vpi: my_task.o
	gcc -o fakefpga.vpi my_task.o $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf *.vpi
	rm -rf transcript

