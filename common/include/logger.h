#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>

typedef enum log_msg_type_enum {
    DEBUG_LOG = 1,
    INFO_LOG = 2,
    WARNING_LOG = 3,
    ERROR_LOG = 4,
    SYSTEM = 15,  // System messages only, like ready (Not logged)
} log_msg_type_t;

typedef struct logger_struct {
    pid_t logger_pid;
    int pipefd;
    
    log_msg_type_t level;
} logger_t;

logger_t* logger_init(const char* log_dir, unsigned const int logger_options);

ssize_t logger_log(logger_t* logger_sender, log_msg_type_t type, const char* message);

void logger_free(logger_t* logger_sender);

#endif

