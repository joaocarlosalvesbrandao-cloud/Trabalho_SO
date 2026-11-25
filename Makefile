all: central

central: central.c central.h comum.h
	gcc central.c -o central

clean:
	rm -f central

