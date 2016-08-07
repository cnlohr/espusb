all : table table2x

table : table.c
	gcc -o $@ $^

table2x : table2x.c
	gcc -o $@ $^

clean :
	rm -rf *.o *~ table table2x
