#ifndef SOCKET_H
#define SOCKET_H

int create_socket( int port );

char* get_socket_ip_address( int socket_fd );
int nonblocking_socket( int socket_fd );

#endif // SOCKET_H