#include <cstring>
#include <iostream>
#include <getopt.h>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <termios.h>
#include <stdlib.h>
#include <mutex>
#include <sys/ioctl.h>
#include "UartControl.hpp"

using namespace std;

int my_fd = -1;

int open_port(string portName)
{
    int fd = -1;

    if (!portName.compare("ttyMSM0")) {
   	  fd = open("/dev/ttyMSM0", O_RDWR | O_NOCTTY | O_NDELAY);
    } else if (!portName.compare("ttyHS1")) {
      fd = open("/dev/ttyHS1", O_RDWR | O_NOCTTY | O_NDELAY);
    } else if (!portName.compare("ttyHS2")) {
      fd = open("/dev/ttyHS2", O_RDWR | O_NOCTTY | O_NDELAY);
    } 
    if (fd < 0){
	return - 1;
    }
    if (fcntl(fd, F_SETFL, 0) < 0) {
       	printf("fcntl failed!\n");
    }
    if (isatty(STDIN_FILENO) == 0) {
	printf("standard input is not a terminal device\n");
    }
    printf("%s done,fd :%d\n",__func__,fd);
    return fd;
}
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;

	if (tcgetattr(fd, &oldtio) != 0)
	{
	   printf("set_opt failed step 1\n");
	   return - 1;
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag		|= CLOCAL | CREAD;
	newtio.c_cflag		&= ~CSIZE;

	if (ioctl(fd, MSM_ENABLE_CLOCK, 2) < 0)
		{
		//LOGE("set_opt failed step 2\n");
		}

	switch (nBits)
		{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
		}
	switch (nEvent)
		{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
		}

	switch (nSpeed)
		{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 19200:
			cfsetispeed(&newtio, B19200);
			cfsetospeed(&newtio, B19200);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		}
	if (nStop == 1)
		{
		newtio.c_cflag		&= ~CSTOPB;
		}
	else if (nStop == 2)
		{
		newtio.c_cflag		|= CSTOPB;
		}

	newtio.c_cc[VTIME]	= 0;
	newtio.c_cc[VMIN]	= 0;
	tcflush(fd, TCIFLUSH);

	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
		{
		printf("com set error");
		return - 1;
	}
    printf("%s done\n",__func__);
	return 0;
}
void get_uart_fd(string uartName,int speed,int bit ,char parity,int stop)
{
	int fd, i;
	std::mutex mtx;
	if (my_fd > 0) {
	   close (my_fd);
	   my_fd = -1;
	}
	if ((fd = open_port(uartName)) < 0)
	{
	   printf("open_port error\n");
		return;
	}

	if ((i = set_opt(fd, speed, bit, parity, stop)) < 0)
	{
		printf("set_opt error\n");
		close(fd);
		return;
	}
    printf("%s called fd:%d\n",__func__,fd);
	
	mtx.lock();
	my_fd = fd;
	mtx.unlock();
}


int send_cmd_to_uart(char *inputCmd)
{
   printf("%s started,my_fd: %d,cmd %s\n",__func__,my_fd,inputCmd);
   std::mutex mtx;
   mtx.lock();
   int 	nwrite = write(my_fd, inputCmd, strlen(inputCmd));
   mtx.unlock();
   
   if (!nwrite) {
	printf("send_cmd_to_uart has complete!\n");
	return 0;
    }
    printf("invalid cmd!\n");
    return -1;
}
