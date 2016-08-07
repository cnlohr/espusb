all : execute_reflash page.mpfs push 

CFLAGS:=-O0 -g

IP?=192.168.4.1

mfsmaker : mfsmaker.c
	gcc -o $@ $^

page.mpfs : mfsmaker page
#	cat to_compress/*.js | gzip -9 > page/compressed.js.gz
	./mfsmaker page page.mpfs

pushtodev : pushtodev.c
	gcc -g -o $@ $^ -lusb-1.0

execute_reflash : execute_reflash.c md5.c
	gcc -g -o $@ $^ -lusb-1.0

push : pushtodev page.mpfs
	./pushtodev $(IP) 65536 page.mpfs

pushusb : pushtodev page.mpfs
	./pushtodev USB 65536 page.mpfs

clean :
	rm -rf mfsmaker page.mpfs pushtodev execute_reflash
