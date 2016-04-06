CC=gcc
CFLAGS=-I. -std=gnu99 -Wall -lcrypto -lcurl
INSTALLDIR=/opt/rfid

all: rfidread

rfidread: relaycontrol.o 
	$(CC) -o rfidread rfidread.c relaycontrol.o $(CFLAGS)

relaycontrol.o: relaycontrol.c
	$(CC) -c relaycontrol.c -lopenssl $(CFLAGS)

clean:
	rm *.o rfidread

install:
	if [ ! -d "$(INSTALLDIR)" ]; then mkdir "$(INSTALLDIR)" && chmod 755 "$(INSTALLDIR)"; fi
	cp rfidread "$(INSTALLDIR)"/
	chmod 755 "$(INSTALLDIR)"/rfidread

kill:
	ps aux | grep "rfidread" | grep -v "grep" | awk '{print $$2}' | xargs kill
