#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include "network.h"
#include "error_fail.h"

int create_socket( int port ) 
{
    int socket_fd;

    //SOCK_STREAM - этот тип обеспечивает последовательный, надежный, ориентированный на установление двусторонней связи поток байтов.
    if ( ( socket_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == 0 ) { 
        handle_error( "Can not create socket!" );
    } 

    struct sockaddr_in address; 
    memset( &address, '0', sizeof( address ) );
    address.sin_family = AF_INET; // ipv4
    address.sin_addr.s_addr = INADDR_ANY; // связать сокет со всеми локальными интерфейсами
    address.sin_port = htons( port ); 
    
    int sock_opt = 1;
    if ( setsockopt( socket_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof( sock_opt ) ) ) {
		handle_error( "Can not set socket options" ); 
    }
    
    /*
        Its possible to set a descriptor so that it is placed in "non-blocking" mode. 
        When placed in non-blocking mode, you never wait for an operation to complete. 
        This is an invaluable tool if you need to switch between many different connected sockets, 
        and want to ensure that none of them cause the program to "lock up."

        If you call "recv()" in non-blocking mode, it will return any data that the 
        system has in it's read buffer for that socket. But, it won't wait for that data. 
        If the read buffer is empty, the system will return from recv() 
        immediately saying ``"Operation Would Block!"''.      
    */
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    if ( bind( socket_fd, ( struct sockaddr * ) &address,  
                                 sizeof( address ) ) <0 ) { 
        handle_error( "Can not bind socket!" );
    } 

    if ( listen( socket_fd, 1 ) < 0 ) {
        handle_error( "Can not start listening server socket!" );
    }

    return socket_fd;
}


char* reverse_dns_lookup( int socket_fd ) {
    struct sockaddr_in peer;
    unsigned int peer_len = sizeof( peer );

    if ( getpeername ( socket_fd, ( struct sockaddr* )( &peer ), &peer_len ) == -1) {
        handle_error( "getpeername() failed" );
    }

    char* hostname = calloc(NI_MAXHOST, sizeof(char));

    getnameinfo(( struct sockaddr* )( &peer ), sizeof(struct sockaddr), hostname, NI_MAXHOST, NULL, 0, 0);

    return hostname;
}

int nonblocking_socket( int socket_fd ) {
    int flags = fcntl( socket_fd, F_GETFL, 0 );
    if ( flags < 0 ) {
        return 1;
    }

    if ( fcntl( socket_fd, F_SETFL, flags | O_NONBLOCK ) ) {
        return 1;
    }

    return 0;
}