#include "server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_socklen_t.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "smtp.h"
#include "tree.h"

#define SESSION_KEY_MAX_LEN 8

#define SERVER_REPLY_MAX_LEN 256
#define SERVER_COMMAND_MAX_LEN 1024

static int open_server_socket(int port) {
    int ret = socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0) {
        debug("Failed to create server socket\n");
        return -1;
    }

    struct sockaddr_in addr = { 0 };
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    if (bind(ret, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        debug("Failed to bind server socket\n");
        close(ret);
        ret = -1;
    } else {
        listen(ret, port);
    }
    return ret;
}

int server_start(int port, const dict_t* config) {
    int server_socket = open_server_socket(port);
    if (server_socket == -1) {
        return -1;
    }

    dict_t sessions;
    fd_set fds, readfds;
    int nfds = server_socket + 1;
    dict_init(&sessions);
    FD_ZERO(&fds);
    FD_ZERO(&readfds);
    FD_SET(server_socket, &fds);

    while (1) {
        readfds = fds;
        if (pselect(nfds, &readfds, NULL, NULL, NULL, NULL) == -1) {
            debug("Failed pselect\n");
            break;  // TODO: error handling
        }

        for (int fd = 0; fd < nfds; ++fd) {
            char session_key[SESSION_KEY_MAX_LEN] = { 0 };
            char reply[SERVER_REPLY_MAX_LEN] = { 0 };
            if (FD_ISSET(fd, &readfds)) {
                debug("%d file descriptor is ready\n", fd);
                if (fd == server_socket) {
                    struct sockaddr_storage client_addr;
                    socklen_t len;
                    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &len);
                    if (client_socket < 0) {
                        continue;  //TODO: error handling
                    }
                    smtp_session_t* session = (smtp_session_t*)malloc(sizeof(smtp_session_t));
                    if (!session) {
                        close(client_socket);
                        continue;
                    }
                    snprintf(session_key, SESSION_KEY_MAX_LEN, "%d", client_socket);
                    if (dict_set(&sessions, session_key, (void*)session) != 0) {
                        free(session);
                        close(client_socket);
                        continue;
                    }
                    FD_SET(client_socket, &fds);
                    if (client_socket + 1 > nfds) {
                        nfds = client_socket + 1;
                    }
                    smtp_start_session(session, reply, SERVER_REPLY_MAX_LEN);
                    send(client_socket, reply, strlen(reply), 0);
                } else {
                    char command[SERVER_COMMAND_MAX_LEN] = { 0 };
                    if (recv(fd, command, SERVER_COMMAND_MAX_LEN, 0) <= 0) {
                        close(fd);
                        FD_CLR(fd, &fds);
                        continue;
                    }
                    snprintf(session_key, SESSION_KEY_MAX_LEN, "%d", fd);
                    smtp_session_t* session = NULL;
                    if (dict_get(&sessions, session_key, (void**)&session) == -1 || !session) {
                        continue;
                    }
                    int ret = smtp_process_command(command, session, reply, SERVER_REPLY_MAX_LEN);
                    send(fd, reply, strlen(reply), 0);
                    if (ret < 0) {
                        close(fd);
                        FD_CLR(fd, &fds);
                        free(session);
                        dict_set(&sessions, session_key, NULL);
                    }
                }
            }
        }
    }

    // Clear sessions
    tree_node_t* node = NULL;
    tree_foreach(sessions.root, node) {
        smtp_session_t* session = (smtp_session_t*)node->value;
        free(session);
    }
    dict_free(&sessions);
    close(server_socket);
    return 0;
}

