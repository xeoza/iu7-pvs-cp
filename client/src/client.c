#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "../include/client.h"
#include "../../common/include/shared_strings.h"
#include "../include/client-fsm.h"

#define BUFSIZE 1000000

#define END_S "\n.\n"


int batch_files_for_processes(mail_files_t* mails, int processes_count, logger_t* logger, int pipeDescr, int is_home_mode) {
    int pids[processes_count];
    int ind = 0;
    logger_log(logger, INFO_LOG, "Started batching of mails\n");

    int start = 0;
    int end = 0;
    int mails_count = mails->count;
    int pack = mails_count / processes_count;
    if (mails_count % processes_count)
        pack++;

    while (end < mails_count) {
        start = end;
        if (end + pack >= mails_count) {
            end = mails_count - 1;
        } else {
            end = end + pack - 1;
        }

        /**
         * Создание нового процесса
         */
        int fork_result = fork();
        if (fork_result == -1) {
            logger_log(logger, INFO_LOG, "Error while creating process\n");
            return -1;
        } else if (fork_result == 0) {
            logger_log(logger, INFO_LOG, "Starting working process\n");
            if (process_mail_files(mails, start, end, logger, pipeDescr, is_home_mode) != 0) {
                logger_log(logger, ERROR_LOG, "Error while processing mail pack\n");
                exit(-1);
            }
            logger_log(logger, INFO_LOG, "Finished working process\n");
            exit(0);
        } else {
            pids[ind] = fork_result;
        }
        ind++;
        end++;
    }

    for (int i = 0; i < ind; i++) {
        int status = 0;
        int opts = 0;
        waitpid(pids[i], &status, opts);
        if (status != 0) {
            logger_log(logger, ERROR_LOG, "Working process finished with error\n");
        }
    }

    logger_log(logger, INFO_LOG, "Finished batch of mails\n");
    return 0;
}

int process_mail_files(mail_files_t* mail_files, int start_ind, int end_ind, logger_t* logger, int pipeDescr,
                       int is_home_mode) {
    logger_log(logger, INFO_LOG, "Processing mails pack\n");
    //Сначала - считать файлы, удалить их из директории и, затем передать их обработчику
    mail_t** mails = malloc(sizeof(mail_t) * (end_ind - start_ind + 1));
    int mail_count = 0;
    for (int i = start_ind; i <= end_ind; i++) {
        int str_count;
        char** mail_text = read_file(mail_files->files[i], &str_count);
        if (mail_text == NULL) {
            logger_log(logger, ERROR_LOG, "Error while reading mail from file\n");
            for (int j = 0; j < mail_count; j++) {
                clear_mail(mails[j]);
            }
            free(mails);
            return -1;
        }

        mail_t* mail = parse_mail(mail_text, str_count, is_home_mode);
        if (mail == NULL) {
            logger_log(logger, ERROR_LOG, "Error while parsing mail file text\n");
            for (int j = 0; j < mail_count; j++) {
                clear_mail(mails[j]);
            }
            clear_mail_text(mail_text);
            free(mails);
            return -1;
        }
        //Освободить считанный текст письма
        clear_mail_text(mail_text);
        mails[mail_count] = mail;
        mail_count++;
    }
    logger_log(logger, INFO_LOG, "Mails pack parsed\n");
    //Затем по идее вызвать селект, создать соединения, обработать их и закрыть
    if (process_mails(mails, mail_count, logger, pipeDescr) != 0) {
        for (int i = 0; i < mail_count; i++) {
            clear_mail(mails[i]);
        }
        free(mails);
        return -1;
    }

    for (int i = 0; i < mail_count; i++) {
        clear_mail(mails[i]);
    }
    free(mails);

    return 0;
}

int process_mails(mail_t** mails, int mail_count, logger_t* logger, int pipeDescr) {
    logger_log(logger, INFO_LOG, "Processing mails\n");
    //Инициализация структур соединений
    int conn_cnt = count_mails_connections(mails, mail_count);
    conn_t** connections = init_connections(mails, mail_count, conn_cnt, logger);
    if (connections == NULL) {
        logger_log(logger, ERROR_LOG, "Can't init connections\n");
        return -1;
    }

    logger_log(logger, INFO_LOG, "Connecting...\n");

    //Инициализация структур select-а
    fd_set saveReadFS, saveWriteFS, readFS, writeFS;
    struct timeval selectTimeout;

    FD_ZERO(&saveReadFS);
    FD_ZERO(&saveWriteFS);
    FD_ZERO(&readFS);
    FD_ZERO(&writeFS);

    FD_SET(pipeDescr, &saveReadFS);


    //Вызовы коннектов
    if (connections_start(connections, conn_cnt, &saveWriteFS, logger) != 0) {
        logger_log(logger, ERROR_LOG, "Can't start connections\n");
        clear_connections(connections, conn_cnt);
        return -1;
    }
    logger_log(logger, INFO_LOG, "Connected\n");

    for (int i = 0; i < conn_cnt; i++) {
        FD_SET(connections[i]->socket, &saveReadFS);
    }

    int sel_run = 1;
    //Селект
    while (sel_run) {
        readFS = saveReadFS;
        writeFS = saveWriteFS;

        selectTimeout.tv_sec = 1;
        selectTimeout.tv_usec = 0;

        if (select(FD_SETSIZE, &readFS, &writeFS, NULL, &selectTimeout) < 0) {
            logger_log(logger, INFO_LOG, "Error in select\n");
            clear_connections(connections, conn_cnt);
            return -1;
        }


        /**
         * Обработка селекта
         * FD_SETSIZE is hardcoded to 1024
         */
        for (int i = 0; i < FD_SETSIZE; i++) {
            /**
             * Макрос FD_ISSET() возвращает
             * ненулевое значение, если указанный файловый дескриптор присутствует в наборе
             * и ноль, если отсутствует
             */
            if (!FD_ISSET(i, &readFS) && !FD_ISSET(i, &writeFS)) {
                continue;
            }

            if (i == pipeDescr) {
                logger_log(logger, INFO_LOG, "Exiting select on close\n");
                clear_connections(connections, conn_cnt);
                return -1;
            }

            conn_t* curr_conn = get_active_connection(connections, conn_cnt, i, logger);
            if (!curr_conn) {
                logger_log(logger, INFO_LOG, "Can't find connection with current socket\n");
                continue;
            }

            if (FD_ISSET(i, &readFS)) {
                if (process_conn_read(curr_conn, &saveWriteFS, logger) != 0) {
                    logger_log(logger, ERROR_LOG, "Error while processing read\n");
                }
            }

            if (FD_ISSET(i, &writeFS)) {
                if (process_conn_write(curr_conn, &saveWriteFS, logger) != 0) {
                    logger_log(logger, ERROR_LOG, "Error while processing write\n");
                }
            }
        }

        if (check_connections_for_finish(connections, conn_cnt) != 0) {
            for (int i = 0; i < conn_cnt; i++) {
                if (connections[i]->state == CLIENT_FSM_ST_FINISH) {
                    char log_str[500];
                    sprintf(log_str, "Successfully send mail to %s\n", connections[i]->to);
                    logger_log(logger, INFO_LOG, log_str);
                } else {
                    char log_str[500];
                    sprintf(log_str, "Failed to send mail to %s\n", connections[i]->to);
                    logger_log(logger, INFO_LOG, log_str);
                }

            }
            sel_run = 0;
        }

        set_connections_need_write(connections, conn_cnt, &saveWriteFS);
    }

    //Очистка
    clear_connections(connections, conn_cnt);
    logger_log(logger, INFO_LOG, "Processing mails finished\n");
    return 0;
}

conn_t** init_connections(mail_t** mails, int mail_count, int conns_count, logger_t* logger) {
    int cci = 0; //Current connection index
    conn_t** connections = malloc(sizeof(conn_t*) * conns_count);
    if (connections == NULL) {
        logger_log(logger, ERROR_LOG, "Cant allocate memory for connections\n");
        return NULL;
    }
    for (int i = 0; i < mail_count; i++) {
        mail_t* curr_mail = mails[i];
        for (int j = 0; j < curr_mail->tos_count; j++) {
            conn_t* new_conn = init_connection(curr_mail, j);
            if (new_conn == NULL) {
                logger_log(logger, ERROR_LOG, "Can't init connection\n");
                for (int k = 0; k < cci; k++) {
                    clear_connection(connections[k]);
                }
                free(connections);
            }
            connections[cci] = new_conn;
            cci++;
        }
    }
    return connections;
}

void clear_connections(conn_t** connections, int conns_count) {
    for (int i = 0; i < conns_count; i++) {
        clear_connection(connections[i]);
    }
    free(connections);
}

int connections_start(conn_t** connections, int conns_count, fd_set* writeFS, logger_t* logger) {
    for (int i = 0; i < conns_count; i++) {
        if (connection_start(connections[i]) != 0) {
            logger_log(logger, ERROR_LOG, "Can't start connection\n");
            client_fsm_step(connections[i]->state, CLIENT_FSM_EV_ERROR, connections[i], writeFS);
            return -1;
        }
    }
    return 0;
}

int count_mails_connections(mail_t** mails, int mail_count) {
    int cnt = 0;
    for (int i = 0; i < mail_count; i++) {
        cnt += mails[i]->tos_count;
    }
    return cnt;
}

conn_t* get_active_connection(conn_t** connections, int conns_count, int socket, logger_t* logger) {
    conn_t* active = NULL;
    for (int i = 0; i < conns_count; i++) {
        if (connections[i]->socket == socket) {
            active = connections[i];
            break;
        }
    }
    return active;
}

int check_connections_for_finish(conn_t** connections, int conns_count) {
    int finished = 1;
    for (int i = 0; i < conns_count; i++) {
        if (connections[i]->state != CLIENT_FSM_ST_INVALID &&
            connections[i]->state != CLIENT_FSM_ST_FINISH &&
            connections[i]->state != CLIENT_FSM_ST_S_ERROR) {
            finished = 0;
        }
    }
    return finished;
}

int set_connections_need_write(conn_t** connections, int conns_count, fd_set* writeFS) {
    FD_ZERO(writeFS);
    for (int i = 0; i < conns_count; i++) {
        if (connections[i]->to_send > 0 &&
            (connections[i]->state != CLIENT_FSM_ST_INVALID &&
             connections[i]->state != CLIENT_FSM_ST_FINISH &&
             connections[i]->state != CLIENT_FSM_ST_S_ERROR)) {
            FD_SET(connections[i]->socket, writeFS);
        }
    }
    return 0;
}

int process_conn_read(conn_t* connection, fd_set* writeFS, logger_t* logger) {
    int read_len = conn_read(connection);
    if (read_len < 0) {
        logger_log(logger, ERROR_LOG, "Error while processing read\n");
        client_fsm_step(connection->state, CLIENT_FSM_EV_ERROR, connection, writeFS);
        return -1;
    } else if (read_len == 0) {
        client_fsm_step(connection->state, CLIENT_FSM_EV_CONNECTION_LOST, connection, writeFS);
        return 0;
    } else {
        int sign_msg = 0;
        while (!sign_msg) {
            int len;
            char* message = try_parse_message_part(&connection->receive_buf, connection->to_receive, &len,
                                                   &(connection->to_receive));
            if (len == -1) {
                logger_log(logger, ERROR_LOG, "Error while processing message\n");
                sign_msg = 1;
                client_fsm_step(connection->state, CLIENT_FSM_EV_ERROR, connection, writeFS);
                return -1;
            } else if (connection->to_receive == 0 && (message == NULL || strlen(message) == 0)) {
                logger_log(logger, ERROR_LOG, "Nothing to receive\n");
                break;
            } else {
                connection->received += len;
                int res = process_message(message);
                if (res < 0) {
                    client_fsm_step(connection->state, CLIENT_FSM_EV_BAD, connection, writeFS);
                    logger_log(logger, ERROR_LOG, message);
                    sign_msg = 1;
                    free(message);

                } else if (res == 0) {
                    //nothing
                } else {
                    logger_log(logger, INFO_LOG, message);
                    client_fsm_step(connection->state, CLIENT_FSM_EV_OK, connection, writeFS);
                    sign_msg = 1;
                    free(message);
                }
            }
        }
    }
    return 0;
}

int process_conn_write(conn_t* connection, fd_set* writeFS, logger_t* logger) {
    int len = conn_write(connection);
    if (len < 0) {
        logger_log(logger, ERROR_LOG, "Error while processing write\n");
        client_fsm_step(connection->state, CLIENT_FSM_EV_ERROR, connection, writeFS);
        return -1;
    }
    if (connection->to_send == 0) {
        //hz
    }
    return 0;
}

int process_message(char* message) {
    int result = 0;
    if (strlen(message) < 4) {
        result = -1;
        return result;
    }

    //Проверка - последнее ли сообщение
    if (message[3] == '-') {
        result = 0;
        return result;
    }


    int responseCode = parse_return_code(message);
    if (responseCode < 200 || responseCode > 600) {
        result = -1;
    } else if (responseCode >= 200 && responseCode < 400) {
        result = 1;
    } else {
        result = -1;
    }
    return result;
}

int conn_read(conn_t* connection) {
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    int len = recv(connection->socket, buf, BUFSIZE, 0);
    if (len < 0) {
        printf("Error in receive\n");
        return len;
    } else if (len == 0) {
        return len;
    } else {
        int new_len = concat_dynamic_strings(&connection->receive_buf, buf, connection->to_receive, len);
        connection->to_receive = new_len;
    }
    return len;
}

int conn_write(conn_t* connection) {
    char buf[BUFSIZE];
    memset(buf, 0, BUFSIZE);
    int send_size = BUFSIZE;
    if (connection->to_send < BUFSIZE) {
        send_size = connection->to_send;
    }
    strncpy(buf, connection->send_buf, send_size);
    int len = send(connection->socket, buf, send_size, 0);
    if (len < 0) {
        printf("Error in send\n");
        return len;
    } else {
        int old_size = connection->to_send;
        int new_size = old_size - len;
        memcpy(connection->send_buf, connection->send_buf + len, sizeof(char) * new_size);
        connection->sended += len;
        connection->to_send -= len;
    }
    return len;
}
