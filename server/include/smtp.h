#ifndef SMTP_H
#define SMTP_H

#include <netinet/in.h>

#include "list.h"

typedef struct smtp_line {
    struct list_head list;
    char data[]; 
} smtp_line_t;

typedef struct smtp_envelope {
    struct list_head* recipients;
    struct list_head* data;
    char* from;
} smtp_envelope_t;

typedef enum smtp_state {
    INITIAL,
    ESTABLISHED,
    MAIL_RCPT,
    MAIL_DATA
} smtp_state_t;

typedef struct smtp_session {
    smtp_state_t state;
    smtp_envelope_t envelope;
    struct in_addr addr;
} smtp_session_t;

int smtp_start_session(smtp_session_t* session, char* reply, size_t len);

void smtp_free_session(smtp_session_t* session);

int smtp_process_command(char* command, smtp_session_t* session, char* reply, size_t len, const char* mail_path);

#endif  // SMTP_H

