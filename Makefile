LDFLAGS=-lusb

all: m4 example

m4: m4api.o m4.c
	gcc -o m4 m4.c m4api.o -lusb

example: m4api.o example.c
	gcc -o example example.c m4api.o -lusb

clean:
	rm -f m4 example m4api.o
