all : controltest

controltest : controltest.o
	gcc -o $@ $^ -lusb-1.0

clean :
	rm -rf *.o *~ controltest
