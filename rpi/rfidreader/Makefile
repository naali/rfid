CC=gcc
CFLAGS=-I. -std=gnu99 -Wall -lcrypto -lcurl -lm -Wstrict-prototypes -Wwrite-strings
INSTALLDIR=/opt/rfid

all: rfidread

cJSON_Utils: cJSON_Utils.c
	$(cc) -c cJSON_Utils.c $(CFLAGS)

cJSON: cJSON.c
	$(cc) -c cJSON.c $(CFLAGS)

rfidread: serialsetup.o cJSON_Utils.o cJSON.o
	$(CC) -o rfidread rfidread.c serialsetup.o cJSON.c cJSON_Utils.c $(CFLAGS)

serialsetup.o: serialsetup.c
	$(CC) -c serialsetup.c -lopenssl $(CFLAGS)

clean:
	rm *.o rfidread

install:
	if [ ! -d "$(INSTALLDIR)" ]; then mkdir "$(INSTALLDIR)" && chmod 755 "$(INSTALLDIR)"; fi
	cp rfidread "$(INSTALLDIR)"/
	chmod 755 "$(INSTALLDIR)"/rfidread

kill:
	ps aux | grep "rfidread" | grep -v "grep" | awk '{print $$2}' | xargs kill
