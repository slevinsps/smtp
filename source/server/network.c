#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "network.h"
#include "error_fail.h"

int create_socket_on_port( int port ) 
{
    int socket_fd;

    if ( ( socket_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == 0 ) { 
        fail_on_error( "Can not create socket!" );
    } 

    struct sockaddr_in address; 
    memset( &address, '0', sizeof( address ) );
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 
    
    int sock_opt = 1;
    if ( setsockopt( socket_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof( sock_opt ) ) ) {
		fail_on_error( "Can not set socket options" ); 
    }
    
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    if ( bind( socket_fd, ( struct sockaddr * ) &address,  
                                 sizeof( address ) ) <0 ) { 
        fail_on_error( "Can not bind socket!" );
    } 

    if ( listen( socket_fd, 1 ) < 0 ) {
        fail_on_error( "Can not start listening server socket!" );
    }

    return socket_fd;
}


char* get_socket_ip_address( int socket_fd ) {
    struct sockaddr_in peer;
    unsigned int peer_len = sizeof( peer );

    if ( getpeername ( socket_fd, ( struct sockaddr* )( &peer ), &peer_len ) == -1) {
        fail_on_error( "getpeername() failed" );
    }

    char* host_ip = inet_ntoa( peer.sin_addr );

    return host_ip;
}

int set_socket_as_nonblocking( int socket_fd ) {
    int flags = fcntl( socket_fd, F_GETFL, 0 );
    if ( flags < 0 ) {
        return 1;
    }

    if ( fcntl( socket_fd, F_SETFL, flags | O_NONBLOCK ) ) {
        return 1;
    }

    return 0;
}