#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "logger.h"
#include "error.h"
#include "debug.h"

#define LOG_MAX_PREFIX_SIZE 32
#define LOG_MAX_FILENAME_SIZE 16
#define LOG_BUFFER_SIZE 128

static void logger_main(int pipefd, const char* log_dir, unsigned long logger_options);

static FILE* logger_open_file(const char* log_dir);

static const char* get_log_msg_type(log_msg_type_t type);

static ssize_t format_message(char* buffer, size_t size, log_msg_type_t level, const char* msg);

logger_t* logger_init(const char* log_dir, unsigned const int logger_options) {
    pid_t pid;
    int pipefd[2];

    debug("Logger: Starting initialization\n");

    if (pipe(pipefd) != 0) {
        debug("Logger: Create pipe failed\n");
        return NULL;
    }

    if ((pid = fork()) == 0) {
        close(pipefd[1]);
        logger_main(pipefd[0], log_dir, logger_options);
        close(pipefd[0]);
        exit(0);
    } else if (pid > 0) {
        close(pipefd[0]);
        debug("Logger: Process forked with pid: %d\n", pid);
        logger_t* logger = (logger_t*) malloc(sizeof(logger_t));
        if (!logger) {
            close(pipefd[1]);
            return NULL;
        }
        logger->logger_pid = pid;
        logger->pipefd = pipefd[1];
        logger->level = logger_options & 0xff; 
        return logger;
    }
    return NULL;
}

ssize_t logger_log(logger_t* logger_sender, log_msg_type_t level, const char* message) {
    if (level < logger_sender->level) {
        return 0;
    }
    
    const size_t size = LOG_MAX_PREFIX_SIZE + strlen(message);
    char* buffer = (char*)malloc(size);
    if (!buffer) {
        return -1;
    }

    ssize_t len = format_message(buffer, size, level, message);
    if (len != -1) {
        len = write(logger_sender->pipefd, buffer, len + 1);
    }
    free(buffer);
    return len;
}

void logger_free(logger_t* logger_sender) {
    close(logger_sender->pipefd);
    free(logger_sender);
}

static const char* get_log_msg_type(log_msg_type_t type) {
    switch (type) {
        case DEBUG_LOG:
            return "DEBUG";
        case INFO_LOG:
            return "INFO";
        case WARNING_LOG:
            return "WARNING";
        case ERROR_LOG:
            return "ERROR";
        default:
            return "NONE";
    }
}

static ssize_t format_message(char* buffer, size_t size, log_msg_type_t level, const char* msg) {
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);

    size_t offset = strftime(buffer, size, "%Y-%m-%d %H:%M:%S\t", tm);
    if (offset == 0) {
        return -1;
    }
    buffer += offset;

    if ((offset += snprintf(buffer, size, "%s\t%s\n", get_log_msg_type(level), msg)) > size) {
        return -1;
    }
    return offset;
}

/**
 * @brief  Log file creation and/or opening
 * @param  logger Logger
 * @param  log_dir Directory with log files
 */
static FILE* logger_open_file(const char* log_dir) {
    char filename[LOG_MAX_FILENAME_SIZE];
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    if (strftime(filename, LOG_MAX_FILENAME_SIZE, "%Y-%m-%d.log", tm) == 0) {
        return NULL;
    }
 
    const size_t path_size = strlen(log_dir) + LOG_MAX_FILENAME_SIZE + 2;
    char* path = (char*)malloc(path_size);
    snprintf(path, path_size, "%s/%s", log_dir, filename);
    debug("Logger: Opening path %s\n", path);
    FILE* log_file = fopen(path, "a");
    free(path);
    return log_file;
}

/**
 * @brief  Logger main loop
 */
static void logger_main(int pipefd, const char* log_dir, unsigned long logger_options) {
    char buffer[LOG_BUFFER_SIZE];
    ssize_t nread = 0;
    debug("Logger: Starting main loop\n");

    while ((nread = read(pipefd, buffer, sizeof(buffer))) > 0) {
        FILE* log_file = logger_open_file(log_dir);
        if (!log_file) {
            break;
        }
        if (fwrite((const void*)buffer, sizeof(char), nread, log_file) < nread) {
            fclose(log_file);
            break;
        }
        fclose(log_file);
    }
}

