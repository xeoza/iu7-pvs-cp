#ifndef CLIENT_H
#define CLIENT_H

#include "dir_worker.h"
#include "parser.h"
#include "smtp_conn.h"
#include "../../common/include/logger.h"

/**
 * Распределение файлов про процессам
 * @param mails
 * @param processes_count
 * @param logger
 * @param pipeDescr
 * @param is_home_mode
 * @return
 */
int batch_files_for_processes(mail_files_t* mails, int processes_count, logger_t* logger, int pipeDescr, int is_home_mode);

/**
 * Обработка пакета писем
 * @param mails
 * @param start_ind
 * @param end_ind
 * @param logger
 * @param pipeDescr
 * @param is_home_mode
 * @return
 */
int process_mail_files(mail_files_t* mails, int start_ind, int end_ind, logger_t* logger, int pipeDescr, int is_home_mode);

/**
 * Отправка письма
 * @param mail_text
 * @param mail_count
 * @param logger
 * @param pipeDescr
 * @return
 */
int process_mails(mail_t** mail_text, int mail_count, logger_t* logger, int pipeDescr);

/**
 * Подсчет числа соединений
 * @param mail_text
 * @param mail_count
 * @param logger
 * @param pipeDescr
 * @return
 */
int count_mails_connections(mail_t** mails, int mail_count);

/**
 * Инициализация соединений
 * @param mails
 * @param mail_count
 * @param conns_count
 * @param logger
 * @return
 */
conn_t** init_connections(mail_t** mails, int mail_count, int conns_count, logger_t* logger);

/**
 * Удаление соединений
 * @param mails
 * @param mail_count
 * @param conns_count
 * @param logger
 * @return
 */
void clear_connections(conn_t** connections, int conns_count);

/**
 * Запуск соединение
 * @param connections
 * @param conns_count
 * @param writeFS
 * @param logger
 * @return
 */
int connections_start(conn_t** connections, int conns_count, fd_set* writeFS, logger_t* logger);

/**
 * Получить соединение с текущим сокетом
 * @param connections
 * @param conns_count
 * @param socket
 * @param logger
 * @return
 */
conn_t* get_active_connection(conn_t** connections, int conns_count, int socket, logger_t* logger);

/**
 * Проверка на завершение соединения по состояниям FSM таких как
 * CLIENT_FSM_ST_INVALID, CLIENT_FSM_ST_FINISH и CLIENT_FSM_ST_S_ERROR
 * @param connections
 * @param conns_count
 * @return
 */
int check_connections_for_finish(conn_t** connections, int conns_count);

/**
 * Получение данных от сервер
 * @param connection
 * @return
 */
int process_conn_read(conn_t* connection, fd_set* writeFS, logger_t* logger);

/**
 * Отправка данных на сервер
 * @param connection
 * @return
 */
int process_conn_write(conn_t* connection, fd_set* writeFS, logger_t* logger);

/**
 * Чтения данных из сокета с помощью recv()
 * @param connection
 * @return
 */
int conn_read(conn_t* connection);

/**
 * Запись данных в сокет с помощью send()
 * @param connection
 * @return
 */
int conn_write(conn_t* connection);

/**
 * Запись текущего сокета в writeFS с помощью FD_SET
 * @param connections
 * @param conns_count
 * @param writeFS
 * @return
 */
int set_connections_need_write(conn_t** connections, int conns_count, fd_set* writeFS);

/**
 * Обработка кода ответа
 * @param message
 * @return
 */
int process_message(char* message);

#endif