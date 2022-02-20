#include "server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "smtp.h"
#include "string_utils.h"
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
    debug("Server socket is open\n");

    dict_t sessions;
    fd_set fds, readfds;
    int nfds = server_socket + 1;
    const char* mail_path = "./mail";
    dict_init(&sessions);
    FD_ZERO(&fds);
    FD_ZERO(&readfds);
    FD_SET(server_socket, &fds);

    dict_get(config, "mail_path", (void**)&mail_path);

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
                if (fd == server_socket) {
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    debug("Incoming connection\n");
                    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &len);
                    if (client_socket < 0) {
                        debug("Failed accept: errno=%d\n", errno);
                        continue;  //TODO: error handling
                    }
                    smtp_session_t* session = (smtp_session_t*)malloc(sizeof(smtp_session_t));
                    session->addr = client_addr.sin_addr;
                    session->envelope.data = NULL;
                    session->envelope.from = NULL;
                    session->envelope.recipients = NULL;
                    session->state = INITIAL;
                    if (!session) {
                        debug("Failed create session\n");
                        close(client_socket);
                        continue;
                    }
                    snprintf(session_key, SESSION_KEY_MAX_LEN, "%d", client_socket);
                    if (dict_set(&sessions, session_key, (void*)session) != 0) {
                        debug("Failed create session\n");
                        free(session);
                        close(client_socket);
                        continue;
                    }
                    FD_SET(client_socket, &fds);
                    if (client_socket + 1 > nfds) {
                        nfds = client_socket + 1;
                    }
                    smtp_start_session(session, reply, SERVER_REPLY_MAX_LEN);
                    debug("New session started\n");
                    send(client_socket, reply, strlen(reply), 0);
                } else {
                    snprintf(session_key, SESSION_KEY_MAX_LEN, "%d", fd);
                    smtp_session_t* session = NULL;
                    if (dict_get(&sessions, session_key, (void**)&session) == -1 || !session) {
                        continue;
                    }

                    char buf[SERVER_COMMAND_MAX_LEN] = { 0 };
                    if (recv(fd, buf, sizeof(buf) - 1, 0) <= 0) {
                        close(fd);
                        FD_CLR(fd, &fds);
                        continue;
                    }

                    for (const char* start = buf; *start != 0;) {
                        char command[SERVER_COMMAND_MAX_LEN] = { 0 };
                        const char* end = strcrlf(start);
                        strncpy(command, start, end - start);
                        start = end;
                        debug("Received command: %s\n", command);
                        int ret = smtp_process_command(command, session, reply, SERVER_REPLY_MAX_LEN, mail_path);
                        send(fd, reply, strlen(reply), 0);
                        debug("Sent reply: %s\n", reply);
                        if (ret < 0) {
                            close(fd);
                            FD_CLR(fd, &fds);
                            free(session);
                            dict_set(&sessions, session_key, NULL);
                            debug("Session finalized\n");
                        }
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

