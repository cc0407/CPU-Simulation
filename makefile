all: main

main: main.c
	gcc -Wpedantic -std=gnu99 main.c -o simcpu

clean:
	rm simcpu