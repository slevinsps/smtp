#ifndef SERVER_H
#define SERVER_H

#include <sys/select.h>

#include "network.h"
#include "client_info.h"
#include "logger.h"

int initialize_server( int port, const char* maildir, logger_t *logger_listener );
void server_update_fds();
int run_server();
void handle_new_connect();
int handle_client_read(client_struct* client);
int handle_client_write(client_struct* client);
void close_server();

typedef struct {
	int server_port;
	char* path_to_maildir;
	char* path_to_logdir;
} config_t;

struct server {
	int server_socket_fd;

	client_struct* clients_list;

	fd_set* read_fds;
	fd_set* write_fds;
	fd_set* exceptions_fds;

	logger_t logger;
	const char* maildir;

	int max_fd;
};

#endif // SERVER_H