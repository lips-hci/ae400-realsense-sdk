#ifndef LINUX_SCAN_H
#define LINUX_SCAN_H

/*Set port number*/
#define SENDER_PORT 8888
#define RECV_PORT   8889

#define INET_ADDR_STR_LEN 128
#define BUF_STR_LEN 200

void udp_broadcast(int timeout_ms = 1500);
void show_AE400_info();

#endif