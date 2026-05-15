#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <termios.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>


int main (int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s path/to/uart/interface\n", argv[0]);
		return 1;
	}
	// O_RDWR to get read and write access
	// O_NOCTTY so it does not become the controlling terminal
	// 
	int fd = open(argv[1], O_RDWR | O_NOCTTY);

	if (fd < 0) {
		fprintf(stderr, "failed to open %s : %s\n", argv[1], strerror(errno));
		close(fd);
		return 1;
	}

	// now we configure UART

	struct termios tty;
	if (tcgetattr(fd, &tty) != 0) {
		perror("tcgetattr");
		close(fd);
		return 1;
	}

	cfmakeraw(&tty);

	// tty.c_cflag contains the settings we need to set which are: baud rate, data bits, parity, and stop bits

	// enable this interface to be a receiver
	tty.c_cflag |= CREAD;

	// this is used to ignore modem control lines
	tty.c_cflag |= CLOCAL;


	// now we set the data size, we first start by clearing data size bits then setting them
	tty.c_cflag |= ~CSIZE;

	// we can set from 5 to 8 data bits by using CS5 .. CS8, i will be choosing 7 data bits
	tty.c_cflag |= CS7;

	// now we set the parity, either odd or even or no parity, i am going with odd parity

	// we set this to enable parity
	tty.c_cflag |= PARENB;
	// then we set this to use odd parity, if we don't it will use even parity
	tty.c_cflag |= PARODD;

	// now we configure stop bits, we can choose between 1 or 2 bits

	// i will configure 2 stop bits by setting CSTOPB, if we don't set it, it will use 1 stop bit
	tty.c_cflag |= CSTOPB;


	// now i will set the baud rate, the baud rate is how many bits can be transferred per second
	// i will set it to B115200, which is 115200 bits per second, 
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	// now after configuring uart i will start sending messages
	unsigned int i = 0;
	while (i < 0x1000) {
		char buf[0x100];
		// we send Hello World on each iteration
		snprintf(buf, 0x100, "Hello World n: %d\n", i);
		// we send our data using write
		ssize_t written = write(fd, buf, strlen(buf));
		// if written returns -1 it means an error happened
		if (written == -1) {
			fprintf(stderr, "write faced an error: %s\n", strerror(errno));
			close(fd);
			return 1;
		}

		// now i receive data, i will be using poll for this
		struct pollfd pfd;
		pfd.fd = fd;
		// we set the event we want to poll, POLLIN for input in this case
		pfd.events = POLLIN;

		// we call poll with nfds == 1 since we have only 1 fd and timeout of 5 seconds in miliseconds
		int ret = poll(&pfd, 1, 5000);
		if (ret == -1) {
			// if poll returns -1 then something went wrong
			perror("poll");
			close(fd);
			return 1;
		} else if (ret == 0) {
			// if it returns 0 then there was a timeout and we didn't receive any data, else something happened
			puts("poll timeout, no data received");
		} else if (pfd.revents & POLLIN) {
			// we received data, we read it and print it to stdout
			ssize_t n = read(fd, buf, sizeof(buf) - 1);
			if (n == -1) {
				fprintf(stderr, "read error happened: %s\n", strerror(errno));
				close(fd);
				return 1;
			}
			buf[n] = 0;
			printf("received %zd bytes\n" ,n);
			printf("data: \"%s\"\n", buf);
		} else if (pfd.events & POLLERR || pfd.events & POLLHUP) {
			// error happened with our fd, we exit gracefully
			fputs("something wrong happened with device", stderr);
			close(fd);
			return 1;
		}
		i++;
	}


}
