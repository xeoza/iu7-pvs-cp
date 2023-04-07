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
#include <signal.h>

#include "debug.h"
#include "logger.h"
#include "smtp.h"
#include "string_utils.h"
#include "tree.h"

#ifdef MODIFY
#include <small/allocator.h>
#include <small/linear.h>
#include <small/pool.h>
#endif

#define SESSION_KEY_MAX_LEN 8

#define SERVER_REPLY_MAX_LEN 256
#define SERVER_COMMAND_MAX_LEN 1024

static int server_running = 0;

static void server_stop(int signal) {
    server_running = 0;
}

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
    sig_t old_sigpipe = signal(SIGPIPE, SIG_IGN);
    sig_t old_sigint = signal(SIGINT, server_stop);
    sig_t old_sigterm = signal(SIGTERM, server_stop);

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
    
    server_running = 1;
    while (server_running) {
        readfds = fds;
        if (pselect(nfds, &readfds, NULL, NULL, NULL, NULL) == -1) {
            logger_log(logger, ERROR_LOG, "Pselect call failed. Stopping server...");
            debug("Failed pselect: errno=%d\n", errno);
            break;
        }

        for (int fd = 0; fd < nfds; ++fd) {
            char session_key[SESSION_KEY_MAX_LEN] = { 0 };
            char reply[SERVER_REPLY_MAX_LEN + 1] = { 0 };
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
#ifdef MODIFY
                    static void* pool = NULL;
                    if (!pool) pool = GetPoolAllocator("session", sizeof(smtp_session_t), 64);
                    smtp_session_t* session = (smtp_session_t*)Allocate(pool, sizeof(smtp_session_t));
#else
                    smtp_session_t* session = (smtp_session_t*)malloc(sizeof(smtp_session_t));
#endif
                    session->addr = client_addr.sin_addr;
                    session->envelope.data = NULL;
                    session->envelope.from = NULL;
                    session->envelope.recipients = NULL;
                    session->state = INITIAL;
		    session->buffer = NULL;
		    session->buffer_size = 0;
                    if (!session) {
                        logger_log(logger, ERROR_LOG, "Session allocation failed. Closing connection...");
                        close(client_socket);
                        continue;
                    }
                    snprintf(session_key, SESSION_KEY_MAX_LEN, "%d", client_socket);
                    if (dict_set(&sessions, session_key, (void*)session) != 0) {
                        logger_log(logger, ERROR_LOG, "Session allocation failed. Closing connection...");
#ifdef MODIFY
                        static void* pool = NULL;
                        if (!pool) pool = GetPoolAllocator("session", sizeof(smtp_session_t), 64);
                        Deallocate(pool, session);
#else
                        free(session);
#endif
 
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

                    char buf[SERVER_COMMAND_MAX_LEN + 1] = { 0 };
                    if (read(fd, buf, SERVER_COMMAND_MAX_LEN) <= 0) {
                        logger_log(logger, WARNING_LOG, "Socket receive failed. Closing connection...");
                        close(fd);
                        FD_CLR(fd, &fds);
                        continue;
                    }
		    // logger_log(logger, INFO_LOG, buf);

                    for (const char* start = buf; *start != 0;) {
                        char* command = NULL;
			size_t command_size = 0;
                        const char* end = strcrlf(start);
                if (!end) {
			    const size_t len = strlen(start);
#ifdef MODIFY2
                static void* linear = NULL;
                if (!linear) linear = GetLinearAllocator("line", 10*1024*1024);
                smtp_line_t* line = Allocate(linear, sizeof(char) * (len + 1) + sizeof(smtp_line_t));
#else
                smtp_line_t* line = malloc(sizeof(char) * (len + 1) + sizeof(smtp_line_t));
#endif
			    if (line) {
			        memset(line, 0, sizeof(smtp_line_t) + sizeof(char) * (len + 1));
			        list_init(&line->list);
				strncpy(line->data, start, len);
				if (!session->buffer) {
                                    session->buffer = &line->list;
				} else {
				    list_add_tail(session->buffer, &line->list);
                                }
				session->buffer_size += len;
                            } else {
			        logger_log(logger, ERROR_LOG, "Failed allocate command buffer");
			    }
                            break;
                        }
			command_size = end - start + session->buffer_size;
			command = malloc(sizeof(char) * (command_size + 1));
			memset(command, 0, command_size + 1);
			if (session->buffer) {
			    smtp_line_t* line = NULL;
			    list_foreach(line, session->buffer, list) {
                                strcat(command, line->data);
			    }
			}
                        strncat(command, start, end - start);
                        start = end + 2;
                        logger_log(logger, INFO_LOG, command);
                        int ret = smtp_process_command(command, session, reply, SERVER_REPLY_MAX_LEN, mail_path);
			free(command);
                        send(fd, reply, strlen(reply), 0);
                        logger_log(logger, INFO_LOG, reply);
                        if (ret < 0) {
                            logger_log(logger, INFO_LOG, "Client sent QUIT command. Closing connection...");
                            close(fd);
                            FD_CLR(fd, &fds);
#ifdef MODIFY
                            static void* pool = NULL;
                            if (!pool) pool = GetPoolAllocator("session", sizeof(smtp_session_t), 64);
                            Deallocate(pool, session);
#else
                            free(session);
#endif
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
#ifdef MODIFY
        static void* pool = NULL;
        if (!pool) pool = GetPoolAllocator("session", sizeof(smtp_session_t), 64);
        Deallocate(pool, session);
#else
        free(session);
#endif
    }
    dict_free(&sessions);
    logger_free(logger);
    close(server_socket);

    signal(SIGPIPE, old_sigpipe);
    signal(SIGINT, old_sigint);
    signal(SIGTERM, old_sigterm);
    return 0;
}

