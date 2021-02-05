#ifndef LINUX_SCAN_H
#define LINUX_SCAN_H

#include <stdio.h>
#include <string>
#include <vector>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>

/*Set port number*/
#define SENDER_PORT 8888
#define RECV_PORT   8889

#define INET_ADDR_STR_LEN 128
#define BUF_STR_LEN 200

void udp_broadcast(int timeout_ms = 2000);
void show_AE400_info();

#endif