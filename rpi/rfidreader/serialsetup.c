#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>

#include "serialsetup.h"

int setup_serial_interface(int fd, int speed, int parity) {
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0) {
		fprintf(stdout, "Error %d while reading TTY attributes.", errno);
		return -1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag |= (CLOCAL | CREAD);
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	tty.c_iflag &= ~IGNBRK;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 0;

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		fprintf(stdout, "Error %d while writing TTY attributes.", errno);
		return -1;
	}

	return 0;
}

int close_serial_interface(int filedesc) {
	close(filedesc);
	return 0;
}

int open_serial_interface(const char * devicename) {
	int filedesc = open(devicename, O_RDWR | O_NOCTTY | O_SYNC);
	return filedesc;
}

