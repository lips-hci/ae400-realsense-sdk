#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <algorithm>
#include <string>
#include <vector>

#include "linux_scan.h"

std::vector<std::string> AE400_IP_Client;
std::vector<std::string> AE400_SN_Client;

char* format_devinfo(const int i)
{
    char devinfo[256];
    int index = AE400_SN_Client[i].rfind( "/" );
    int len = 0;
    len += sprintf( devinfo + len, "IP address  = %s\n", AE400_IP_Client[i].c_str() );
    len += sprintf( devinfo + len, "Version     = %s\n", AE400_SN_Client[i].substr( index + 1 ).c_str() );
    len += sprintf( devinfo + len, "Description = %s\n", AE400_SN_Client[i].substr( 0, index ).c_str() );
    return strdup(devinfo);
}

void get_ifaddrs_name(const char *ifaddrs[], int *ifnum)
{
    struct ifaddrs *_ifaddr, *ifa;
    int family, s, n = 0;
    char host[NI_MAXHOST];

    if (getifaddrs(&_ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later */

    for (ifa = _ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
           form of the latter for the common families) */

        //printf("%-8s %s (%d)\n",
        //       ifa->ifa_name,
        //       (family == AF_PACKET) ? "AF_PACKET" :
        //       (family == AF_INET) ? "AF_INET" :
        //       (family == AF_INET6) ? "AF_INET6" : "???",
        //       family);

        /* For an AF_INET* interface address, display the address */

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                continue;
            }
            if (!strcmp(host, "127.0.0.1") || !strcmp(host, "0.0.0.0")) {
                continue; //Bypass ZERO or loopback interface
            }
            //printf("%d\tdev:<%s>\t\taddress:<%s>\n", n, ifa->ifa_name, host);
            ifaddrs[n++] = strdup(host);
        }
    }

    if (_ifaddr)
        freeifaddrs(_ifaddr);

    *ifnum = n;
    return;
}

void udp_broadcast_client(int timeout_ms)
{
    int sockfd, yes = 1;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t addrlen = sizeof( client_addr );
    ssize_t size;
    char ip4[INET_ADDR_STR_LEN] = {0}, buf[BUF_STR_LEN] = {0};
    struct timeval tv;
    fd_set readfds;

    //Create an IPv4 and UDP socket
    if ( ( sockfd = socket( PF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror( "socket" );
        return ;
    }

    //Set the struct of sockaddr_in
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons( RECV_PORT );
    serv_addr.sin_addr.s_addr = htonl( INADDR_ANY ); /*All the host*/

    if ( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes ) ) < 0 )
    {
        perror( "setsockopt" );
        return ;
    }

    if ( bind( sockfd, ( struct sockaddr * )&serv_addr, sizeof( struct sockaddr_in ) ) != 0 )
    {
        perror( "bind" );
        return ;
    }

    FD_ZERO( &readfds );
    FD_SET( sockfd, &readfds );
    tv.tv_sec = (timeout_ms / 1000);
    tv.tv_usec = (timeout_ms - (tv.tv_sec*1000))*1000;
    select( sockfd + 1, &readfds, NULL, NULL, &tv );
    if ( FD_ISSET( sockfd, &readfds ) )
    {
        size = recvfrom( sockfd, buf, BUF_STR_LEN, 0, ( struct sockaddr * )&client_addr, &addrlen );
        if ( size < 0 )
        {
            perror( "recvfrom" );
            close( sockfd );
            return;
        }

        inet_ntop( AF_INET, &client_addr.sin_addr, ip4, INET_ADDRSTRLEN );
        auto dev = std::find(AE400_IP_Client.begin(), AE400_IP_Client.end(), ip4);
        if (dev == AE400_IP_Client.end())
        {
            AE400_IP_Client.push_back( ip4 );
            AE400_SN_Client.push_back( buf );
            int new_found = AE400_IP_Client.size() - 1;
            printf("%s\n", format_devinfo(new_found));
        }
    }

    close( sockfd );
    return;
}

void _udp_broadcast(const char* hostaddr, const int timeout_ms)
{
    int sockfd, so_broadcast = 1;
    struct sockaddr_in server_addr, client_addr;
    struct hostent *he;

    /*Create an IPv4 UDP socket*/
    if ( ( sockfd = socket( PF_INET, SOCK_DGRAM, 0 ) ) < 0 )
    {
        perror( "socket" );
        return;
    }

    /*SO_BROADCAST: broadcast attribute*/
    if ( setsockopt( sockfd, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof( so_broadcast ) ) < 0 )
    {
        perror( "setsockopt" );
        return;
    }

    if ( ( he = gethostbyname(hostaddr) ) == NULL ) {
        perror( "gethostbyname" );
        return;
    }

    server_addr.sin_family = AF_INET; /*IPv4*/
    server_addr.sin_port = htons( INADDR_ANY ); /*All the port*/
    server_addr.sin_addr.s_addr = inet_addr(hostaddr); /*Broadcast address*/

    if ( ( bind( sockfd, ( struct sockaddr* )&server_addr, sizeof( struct sockaddr ) ) ) != 0 )
    {
        perror( "bind" );
        return;
    }

    client_addr.sin_family = AF_INET; /*IPv4*/
    client_addr.sin_port = htons( SENDER_PORT ); /*Set port number*/
    client_addr.sin_addr.s_addr = htonl( INADDR_BROADCAST ); /*The broadcast address*/

    /*Use sendto() to send messages to client*/
    /*sendto() doesn't need to be connected*/
    if ( ( sendto( sockfd, "AE400-echo", sizeof( "AE400-echo" ), 0, ( struct sockaddr* )&client_addr, sizeof( struct sockaddr ) ) ) < 0 )
        perror( "sendto" );
    else
        udp_broadcast_client(timeout_ms);

    close( sockfd );
    return;
}

void udp_broadcast(int timeout_ms)
{
    static const char *ifaddrs[32] = {""};
    static int ifnum = 32;

    if (ifaddrs[0] == "") {
        get_ifaddrs_name(ifaddrs, &ifnum);
    }

    //ifnum will be updated to how many of network interface/adapter on this host
    for (int i = 0; i < ifnum; i++) {
        _udp_broadcast(ifaddrs[i], timeout_ms);
    }

    return;
}

void show_AE400_info()
{
    AE400_IP_Client.clear();
    AE400_SN_Client.clear();

    printf( "====== Scan Result ======\n\n" );
    for ( int i = 0; i < 4; i++) {
        udp_broadcast();
    }
    printf( "====== Scan End    ======\n\n" );
}