#ifndef SOCKET_H
#define SOCKET_H

int create_socket_on_port( int port );

char* get_socket_ip_address( int socket_fd );
int set_socket_as_nonblocking( int socket_fd );

#endif // SOCKET_H