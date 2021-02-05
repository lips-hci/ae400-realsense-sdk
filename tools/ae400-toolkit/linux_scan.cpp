#include <string.h>
#include <algorithm>

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

void udp_broadcast_clinet(int timeout_ms)
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
    tv.tv_sec = (timeout_ms / 1000); //timeout default is 2000ms
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
            printf("%s", format_devinfo(new_found));
        }
    }

    close( sockfd );
    return;
}

void udp_broadcast(int timeout_ms)
{
    int sockfd, so_broadcast = 1;
    struct sockaddr_in server_addr, client_addr;

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

    server_addr.sin_family = AF_INET; /*IPv4*/
    server_addr.sin_port = htons( INADDR_ANY ); /*All the port*/
    server_addr.sin_addr.s_addr = htonl( INADDR_BROADCAST ); /*Broadcast address*/

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
        udp_broadcast_clinet(timeout_ms);

    close( sockfd );
    return;
}

void show_AE400_info()
{
    AE400_IP_Client.clear();
    AE400_SN_Client.clear();

    printf( "====== Scan Result ======\n\n" );
    for ( int i = 0; i < 10; i++) {
        udp_broadcast();
    }
    printf( "====== Scan End    ======\n\n" );
}
