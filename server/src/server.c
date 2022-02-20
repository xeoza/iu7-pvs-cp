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
#include "logger.h"
#include "smtp.h"
#include "string_utils.h"
#include "tree.h"

#define SESSION_KEY_MAX_LEN 8

#define SERVER_REPLY_MAX_LEN 256
#define SERVER_COMMAND_MAX_LEN 1024

static int open_server_socket(int port) {
    int ret = socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0) {
        debug("Failed to create server socket: errno=%d\n", errno);
        return -1;
    }

    struct sockaddr_in addr = { 0 };
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    if (bind(ret, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        debug("Failed to bind server socket: errno=%d\n", errno);
        close(ret);
        ret = -1;
    } else {
        listen(ret, port);
    }
    return ret;
}

int server_start(int port, const dict_t* config) {
    const char* mail_path = "./mail";
    const char* logs_path = "./logs";
    dict_get(config, "mail_path", (void**)&mail_path);
    dict_get(config, "logs_path", (void**)&logs_path);

    logger_t* logger = logger_init(logs_path, INFO_LOG);
    if (!logger) {
        return -1;
    }

    int server_socket = open_server_socket(port);
    if (server_socket == -1) {
        logger_log(logger, ERROR_LOG, "Server socket open failed");
        logger_free(logger);
        return -1;
    }
    debug("Server socket is open\n");

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
            logger_log(logger, ERROR_LOG, "Pselect call failed. Stopping server...");
            debug("Failed pselect: errno=%d\n", errno);
            break;
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
                        logger_log(logger, ERROR_LOG, "Accept call failed. Incoming connection declined.");
                        debug("Failed accept: errno=%d\n", errno);
                        continue;
                    }
                    smtp_session_t* session = (smtp_session_t*)malloc(sizeof(smtp_session_t));
                    session->addr = client_addr.sin_addr;
                    session->envelope.data = NULL;
                    session->envelope.from = NULL;
                    session->envelope.recipients = NULL;
                    session->state = INITIAL;
                    if (!session) {
                        logger_log(logger, ERROR_LOG, "Session allocation failed. Closing connection...");
                        close(client_socket);
                        continue;
                    }
                    snprintf(session_key, SESSION_KEY_MAX_LEN, "%d", client_socket);
                    if (dict_set(&sessions, session_key, (void*)session) != 0) {
                        logger_log(logger, ERROR_LOG, "Session allocation failed. Closing connection...");
                        free(session);
                        close(client_socket);
                        continue;
                    }
                    FD_SET(client_socket, &fds);
                    if (client_socket + 1 > nfds) {
                        nfds = client_socket + 1;
                    }
                    smtp_start_session(session, reply, SERVER_REPLY_MAX_LEN);
                    logger_log(logger, INFO_LOG, "New session started");
                    send(client_socket, reply, strlen(reply), 0);
                } else {
                    snprintf(session_key, SESSION_KEY_MAX_LEN, "%d", fd);
                    smtp_session_t* session = NULL;
                    if (dict_get(&sessions, session_key, (void**)&session) == -1 || !session) {
                        logger_log(logger, WARNING_LOG, "Unknown client. No active sessions found.");
                        close(fd);
                        FD_CLR(fd, &fds);
                        continue;
                    }

                    char buf[SERVER_COMMAND_MAX_LEN] = { 0 };
                    if (recv(fd, buf, sizeof(buf) - 1, 0) <= 0) {
                        logger_log(logger, WARNING_LOG, "Socker receive failed. Closing connection...");
                        close(fd);
                        FD_CLR(fd, &fds);
                        continue;
                    }

                    for (const char* start = buf; *start != 0;) {
                        char command[SERVER_COMMAND_MAX_LEN] = { 0 };
                        const char* end = strcrlf(start);
                        strncpy(command, start, end - start);
                        start = end;
                        logger_log(logger, INFO_LOG, command);
                        int ret = smtp_process_command(command, session, reply, SERVER_REPLY_MAX_LEN, mail_path);
                        send(fd, reply, strlen(reply), 0);
                        logger_log(logger, INFO_LOG, reply);
                        if (ret < 0) {
                            logger_log(logger, INFO_LOG, "Client sent QUIT command. Closing connection...");
                            close(fd);
                            FD_CLR(fd, &fds);
                            free(session);
                            dict_set(&sessions, session_key, NULL);
                            logger_log(logger, INFO_LOG, "Session finished");
                        }
                    }
                }
            }
        }
    }
 
    logger_log(logger, INFO_LOG, "Stopping server...");
    // Clear sessions
    tree_node_t* node = NULL;
    tree_foreach(sessions.root, node) {
        smtp_session_t* session = (smtp_session_t*)node->value;
        free(session);
    }
    dict_free(&sessions);
    logger_free(logger);
    close(server_socket);
    return 0;
}

