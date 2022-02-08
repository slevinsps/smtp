#ifndef SOCKET_H
#define SOCKET_H

int create_socket( int port );

char* reverse_dns_lookup( int socket_fd );
int nonblocking_socket( int socket_fd );

#endif // SOCKET_H