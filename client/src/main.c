/** 
 * @file
 * @brief Основная точка входа
 */
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../common/include/logger.h"
#include "../../common/include/error.h"
#include "../include/dir_helper.h"
#include "../include/client.h"
#include "../include/dir_worker.h"
#include "../include/client-opts.h"

#define BASE_LOG_DIR "./build/log"
#define BASE_MAILDIR "./work_mails"
#define BASE_PROC_COUNT 1
#define CHECK_PAUSE 1
#define BASE_IS_HOME_MODE 1

/**
 * Флаг запуска процесса
 */
static volatile int run = 1;

static logger_t* logger;
/**
 * Массив pipefd используется для возврата двух файловых описателей, указывающих на концы канала. 
 * pipefd[0] указывает на конец канала для чтения. 
 * pipefd[1] указывает на конец канала для записи. 
 */
static int pipefd[2] = {0, 0};

/**
 * Завершение работы в случае прерывания
 * @param sig
 */
static void close_handler(int signal);

int main_loop();

/**
 * @brief Структура опций клиента
 */
typedef struct options_struct {
    const char* log_dir;            ///< Каталог для хранения файлов журнала
    const char* maildir;            ///< Каталог с локальной почтой

    long int home_mode;             ///< Флаг локального запуска
    long int proc_count;            ///< Количество рабочих процессов
} options_t;

options_t client_options;

/**
 * Заполнение входных параметров из командной строки
 * @return
 */
options_t fill_options() {
    options_t options = {.home_mode = OPT_VALUE_HOME_MODE};
    if (HAVE_OPT(PROC_COUNT)) {
        options.proc_count = (long int) OPT_ARG(PROC_COUNT);
    } else {
        options.proc_count = BASE_PROC_COUNT;
    }
    if (HAVE_OPT(MAIL_DIR)) {
        options.maildir = OPT_ARG(MAIL_DIR);
    } else {
        options.maildir = BASE_MAILDIR;
    }
    if (HAVE_OPT(LOG_DIR)) {
        options.log_dir = OPT_ARG(LOG_DIR);
    } else {
        options.log_dir = BASE_LOG_DIR;
    }

    return options;
}

/**
 * Валидация входных параметров
 * @param options
 * @return
 */
int validate_options(options_t options) {
    if (create_dir_if_not_exists(options.log_dir) < 0)
        return -1;
    return 0;
}


int main(int argc, char** argv) {
    optionProcess(&clientOptions, argc, argv);
    client_options = fill_options();
    if (validate_options(client_options) < 0) {
        exit_on_error("validate_options");
    }

    int ret;

    if (pipe(pipefd) < 0) {
        printf("Can't init pipe\n");
        return -1;
    }

    /**
     * Прерывание процесса комбинацией CTRL-C
     */
    signal(SIGINT, close_handler);
    /**
     * Завершение программы через kill()
     */
    signal(SIGTERM, close_handler);

    logger = logger_init(client_options.log_dir, INFO_LOG);
    if (!logger) {
        printf("Can't init logger\n");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    ret = main_loop();

    logger_free(logger);
    close(pipefd[0]);
    close(pipefd[1]);
    return ret;
}

/**
 * @brief Основной цикл при флаге запуска процесса равному 1
 */
int main_loop() {
    while (run) {
        mail_files_t* mails = check_directory(client_options.maildir);
        if (mails == NULL) {
            logger_log(logger, ERROR_LOG, "Error while checking mails directory\n");
            run = 0;
        } else if (mails->count == 0) {
            sleep(CHECK_PAUSE);
        } else {
            logger_log(logger, INFO_LOG, "Some mails in directory\n");
            if (batch_files_for_processes(mails, client_options.proc_count, logger, pipefd[0],
                                          client_options.home_mode) != 0) {
                logger_log(logger, ERROR_LOG, "Error while processing mails\n");
                run = 0;
                clear_mail_files(mails);
                break;
            }

            //Удаление писем
            logger_log(logger, INFO_LOG, "Deleting processed mails\n");
            for (int i = 0; i < mails->count; i++) {
                if (remove(mails->files[i]) != 0) {
                    logger_log(logger, ERROR_LOG, "Error while deleting mail file\n");
                } else {
                    logger_log(logger, INFO_LOG, "Mail file deleted\n");
                }
            }
            sleep(CHECK_PAUSE);
        }
        clear_mail_files(mails);
    }
    return 0;
}

static void close_handler(int sig) {
    write(pipefd[1], "END", 3);
    run = 0;
    return;
}