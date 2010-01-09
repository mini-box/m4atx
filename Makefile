LDFLAGS=-lusb

m4: m4api.o m4.c
	gcc -o m4 m4.c m4api.o -lusb

clean:
	rm -f m4 m4api.o
