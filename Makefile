all: args_test
args_test: args_test.o
	gcc -o $@ $<
args_test.o: args_test.c args.h Makefile
	gcc -Wall -c -g -o $@ $<
clean:
	rm -f args_test args_test.o

