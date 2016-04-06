#ifndef relaycontrol
#define relaycontrol

#include <termios.h>
#include <unistd.h>
#include <errno.h>

int setup_serial_interface(int fd, int speed, int parity);
int open_serial_interface(char * devicename);
int close_serial_interface(int filedesc);

#endif


