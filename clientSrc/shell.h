#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <string.h> 
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/epoll.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#define __USE_BSD
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>

int shell_unix();
int init_shell(char *addr, int port);