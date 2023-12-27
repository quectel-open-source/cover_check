#ifndef __UART_CONTROL_H__
#define __UART_CONTROL_H__
#define MSM_ENABLE_CLOCK		0x5441
int open_port(int comport);
int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);
void get_uart_fd(std::string uartName,int speed,int bit ,char parity,int stop);
int send_cmd_to_uart(char *inputCmd);
#endif

