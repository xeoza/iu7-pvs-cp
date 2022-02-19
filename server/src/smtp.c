#include "smtp.h"

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "list.h"
#include "string_utils.h"

#define REGEX_ATOM "([0-9a-zA-Z\\-]+)"
#define REGEX_DOMAIN "(" REGEX_ATOM "(\\." REGEX_ATOM ")*)"
#define REGEX_MAIL "(" REGEX_DOMAIN "@" REGEX_DOMAIN ")"

static void envelope_free(smtp_envelope_t* envelope) {
    if (envelope->recipients) {
        for (struct list_head *head = envelope->recipients, *next = head->next; head != NULL;) {
            free(container_of(head, smtp_line_t, list));
            if (next != envelope->recipients) {
                head = next;
            } else {
                head = NULL;
            }
        }
    }
    envelope->recipients = NULL;
    if (envelope->data) {
        free(envelope->data);
    }
    envelope->data = NULL;
    if (envelope->from) {
        free(envelope->from);
    }
    envelope->from = NULL;
}

static void session_reset(smtp_session_t* session, smtp_state_t state) {
    session->state = state;
    envelope_free(&session->envelope);
}

int smtp_start_session(smtp_session_t* session, char* reply, size_t len) {
    session_reset(session, INITIAL);
    snprintf(reply, len, "220\r\n");
    return 0;
}

void smtp_free_session(smtp_session_t* session) {
    session_reset(session, INITIAL);
}

static int smtp_ehlo(const char* command, smtp_session_t* session, char* reply, size_t len) {
    session_reset(session, ESTABLISHED);
    snprintf(reply, len, "250\r\n");
    return 0;
}

static int smtp_mail(const char* command, smtp_session_t* session, char* reply, size_t len) {
    regex_t regex;
    regmatch_t matches[1];
    regcomp(&regex, "^MAIL FROM:<" REGEX_MAIL ">.*$", REG_EXTENDED);
    if (regexec(&regex, command, 1, matches, 0) != 0) {
        snprintf(reply, len, "500\r\n");
    } else if (session->state != ESTABLISHED) {
        snprintf(reply, len, "503\r\n");
    } else {
        const size_t from_len = matches[0].rm_eo - matches[0].rm_so;
        session->state = MAIL_RCPT;
        session->envelope.from = (char*)malloc(from_len * sizeof(char));
        if (session->envelope.from) {
            strncpy(session->envelope.from, command + matches[0].rm_so, from_len);
            snprintf(reply, len, "250 OK\r\n");
        } else {
            snprintf(reply, len, "500\r\n");
        }
    }
    regfree(&regex);
    return 0;
}

static int smtp_rcpt(const char* command, smtp_session_t* session, char* reply, size_t len) {
    regex_t regex;
    regmatch_t matches[1];
    regcomp(&regex, "^RCPT TO:<" REGEX_MAIL ">$", REG_EXTENDED);
    if (regexec(&regex, command, 1, matches, 0) != 0) {
        snprintf(reply, len, "500\r\n");
    } else if (session->state != MAIL_RCPT) {
        snprintf(reply, len, "503\r\n");
    } else {
        const size_t recipient_len = matches[0].rm_eo - matches[0].rm_so;
        smtp_line_t* recipient = (smtp_line_t*)malloc(sizeof(smtp_line_t) + recipient_len + 1);
        if (recipient) {
            list_init(&recipient->list);
            strncpy(recipient->data, command + matches[0].rm_so, recipient_len);
            if (!session->envelope.recipients) {
                session->envelope.recipients = &recipient->list;
            } else {
                list_add(session->envelope.recipients, &recipient->list);
            }
            snprintf(reply, len, "250\r\n");
        } else {
            snprintf(reply, len, "500\r\n");
        }
    }
    regfree(&regex);
    return 0;
}

static int smtp_data(const char* command, smtp_session_t* session, char* reply, size_t len) {
    regex_t regex;
    regcomp(&regex, "^DATA$", REG_EXTENDED);
    if (regexec(&regex, command, 0, NULL, 0) != 0) {
        snprintf(reply, len, "500\r\n");
    } else if (session->state != MAIL_RCPT) {
        snprintf(reply, len, "503\r\n");
    } else {
        session->state = MAIL_DATA;
        snprintf(reply, len, "354\r\n");
    }
    regfree(&regex);
    return 0;
}

static int envelope_save(const smtp_envelope_t* envelope, const char* mail_path) {
    char date[10];
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d", tm);

    smtp_line_t* recipient = NULL;
    list_foreach(recipient, envelope->recipients, list) {
        char file_path[512];
        FILE* file;
        snprintf(file_path, sizeof(file_path), "%s/%s/%s_%s", mail_path, recipient->data, date, envelope->from);
 
        if (!(file = fopen(file_path, "w"))) {
            return -1;
        }
        smtp_line_t* line = NULL;
        list_foreach(line, envelope->data, list) {
            fputs(line->data, file);
        }
        fclose(file);
    }
    return 0;
}

static int smtp_read_data(const char* command, smtp_session_t* session, char* reply, size_t len, const char* mail_path) {
    if (strcmp(".", command) == 0) {
        if (envelope_save(&session->envelope, mail_path)) {
            snprintf(reply, len, "500\r\n");
        } else {
            snprintf(reply, len, "250 OK\r\n");
        }
        session_reset(session, ESTABLISHED);
    } else {
        const size_t data_len = strlen(command);
        smtp_line_t* data = (smtp_line_t*)malloc(sizeof(smtp_line_t) + data_len + 1);
        if (data) {
            list_init(&data->list);
            strncpy(data->data, command, data_len);
            if (!session->envelope.data) {
                session->envelope.data = &data->list;
            } else {
                list_add(session->envelope.data, &data->list);
            }
            snprintf(reply, len, "354\r\n");
        } else {
            snprintf(reply, len, "500\r\n");
        }
    }
    return 0;
}

static int smtp_reset(char* command, smtp_session_t* session, char* reply, size_t len) {
    if (strcmp("RSET\r\n", command) != 0) {
        snprintf(reply, len, "500\r\n");
    } else {
        session_reset(session, session->state == INITIAL ? INITIAL : ESTABLISHED);
        snprintf(reply, len, "250 OK\r\n");
    }
    return 0;
}

static int smtp_quit(char* command, smtp_session_t* session, char* reply, size_t len) {
    if (strcmp("QUIT\r\n", command) != 0) {
        snprintf(reply, len, "500\r\n");
    } else {
        smtp_free_session(session);
        snprintf(reply, len, "221 OK\r\n");
    }
    return -1;
}



int smtp_process_command(char* command, smtp_session_t* session, char* reply, size_t len, const char* mail_path) {
    if (session->state == MAIL_DATA) {
        return smtp_read_data(command, session, reply, len, mail_path);
    }
    command = strtrim(command);
    if (strstartswith(command, "EHLO")) {
        return smtp_ehlo(command, session, reply, len);
    } else if (strstartswith(command, "MAIL")) {
        return smtp_mail(command, session, reply, len);
    } else if (strstartswith(command, "RCPT")) {
        return smtp_rcpt(command, session, reply, len);
    } else if (strstartswith(command, "DATA")) {
        return smtp_data(command, session, reply, len);
    } else if (strstartswith(command, "RSET")) {
        return smtp_reset(command, session, reply, len);
    } else if (strstartswith(command, "QUIT")) {
        return smtp_quit(command, session, reply, len);
    } else if (strstartswith(command, "VRFY") || strstartswith(command, "VERIFY")) {
    }
    snprintf(reply, len, "500\r\n");
    return 0;
}

