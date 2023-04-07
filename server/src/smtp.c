#include "smtp.h"

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "list.h"
#include "maildir.h"
#include "string_utils.h"

#define REGEX_ATOM "([0-9a-zA-Z\\-]+)"
#define REGEX_DOMAIN "(" REGEX_ATOM "(\\." REGEX_ATOM ")*)"
#define REGEX_MAIL "(" REGEX_DOMAIN "@" REGEX_DOMAIN ")"

static void free_smtp_lines(struct list_head* lines) {
    for (struct list_head *head = lines, *next = head->next; head != NULL;) {
        free(container_of(head, smtp_line_t, list));
        if (next != lines) {
            head = next;
	    next = head->next;
        } else {
            head = NULL;
        }
    }
}

static void envelope_free(smtp_envelope_t* envelope) {
    if (envelope->recipients) {
        free_smtp_lines(envelope->recipients);
    }
    envelope->recipients = NULL;
    if (envelope->data) {
        free_smtp_lines(envelope->data);
    }
    envelope->data = NULL;
    if (envelope->from) {
        free(envelope->from);
    }
    envelope->from = NULL;
}

static void buffer_reset(smtp_session_t* session) {
    if (session->buffer) {
        free_smtp_lines(session->buffer);
    }
    session->buffer = NULL;
    session->buffer_size = 0;
}

void smtp_session_reset(smtp_session_t* session, smtp_state_t state) {
    buffer_reset(session);
    session->state = state;
    envelope_free(&session->envelope);
}

int smtp_start_session(smtp_session_t* session, char* reply, size_t len) {
    smtp_session_reset(session, INITIAL);
    snprintf(reply, len, "220\r\n");
    return 0;
}

void smtp_free_session(smtp_session_t* session) {
    smtp_session_reset(session, INITIAL);
}

static int smtp_ehlo(const char* command, smtp_session_t* session, char* reply, size_t len) {
    smtp_session_reset(session, ESTABLISHED);
    snprintf(reply, len, "250\r\n");
    return 0;
}

static int smtp_mail(const char* command, smtp_session_t* session, char* reply, size_t len) {
    regex_t regex;
    regmatch_t matches[2];
    regcomp(&regex, "^MAIL FROM:<" REGEX_MAIL ">.*$", REG_EXTENDED | REG_ICASE);
    if (regexec(&regex, command, 2, matches, 0) != 0) {
        snprintf(reply, len, "500\r\n");
    } else if (session->state != ESTABLISHED) {
        snprintf(reply, len, "503\r\n");
    } else {
        const size_t from_len = matches[1].rm_eo - matches[1].rm_so;
        session->state = MAIL_RCPT;
        session->envelope.from = (char*)malloc(sizeof(char) * from_len);
        memset(session->envelope.from, 0, sizeof(char) * from_len);
        if (session->envelope.from) {
            strncpy(session->envelope.from, command + matches[1].rm_so, from_len);
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
    regmatch_t matches[2];
    regcomp(&regex, "^RCPT TO:<" REGEX_MAIL ">$", REG_EXTENDED | REG_ICASE);
    if (regexec(&regex, command, 2, matches, 0) != 0) {
        snprintf(reply, len, "500\r\n");
    } else if (session->state != MAIL_RCPT) {
        snprintf(reply, len, "503\r\n");
    } else {
        const size_t recipient_len = matches[1].rm_eo - matches[1].rm_so;
        smtp_line_t* recipient = (smtp_line_t*)malloc(sizeof(smtp_line_t) + sizeof(char) * (recipient_len + 1));
        memset(recipient, 0, sizeof(smtp_line_t) + sizeof(char) * (recipient_len + 1));
        if (recipient) {
            list_init(&recipient->list);
            strncpy(recipient->data, command + matches[1].rm_so, recipient_len);
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
    regcomp(&regex, "^DATA$", REG_EXTENDED | REG_ICASE);
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
    smtp_line_t* recipient = NULL;
    return 0;
    list_foreach(recipient, envelope->recipients, list) {
        char file_name[MAILDIR_NAME_LEN] = { 0 };
        int fd = maildir_tmp(mail_path, recipient->data, file_name, sizeof(file_name));
        if (fd < 0) {
            return -1;
        }
        smtp_line_t* line = NULL;
        list_foreach(line, envelope->data, list) {
            write(fd, line->data, strlen(line->data));
        }
        close(fd);
        maildir_new(mail_path, recipient->data, file_name);
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
        smtp_session_reset(session, ESTABLISHED);
    } else {
        const size_t data_len = strlen(command);
        smtp_line_t* data = (smtp_line_t*)malloc(sizeof(smtp_line_t) + sizeof(char) * (data_len + 1));
        if (data) {
            memset(data, 0, sizeof(smtp_line_t) + sizeof(char) * (data_len + 1));
            list_init(&data->list);
            strncpy(data->data, command, data_len);
            if (!session->envelope.data) {
                session->envelope.data = &data->list;
            } else {
                list_add(session->envelope.data, &data->list);
            }
            // snprintf(reply, len, "354\r\n");
        } else {
            snprintf(reply, len, "500\r\n");
        }
    }
    return 0;
}

static int smtp_reset(char* command, smtp_session_t* session, char* reply, size_t len) {
    regex_t regex;
    regcomp(&regex, "^RSET$", REG_EXTENDED | REG_ICASE);
    if (regexec(&regex, command, 0, NULL, 0) != 0) {
        snprintf(reply, len, "500\r\n");
    } else {
        smtp_session_reset(session, session->state == INITIAL ? INITIAL : ESTABLISHED);
        snprintf(reply, len, "250 OK\r\n");
    }
    return 0;
}

static int smtp_quit(char* command, smtp_session_t* session, char* reply, size_t len) {
    regex_t regex;
    regcomp(&regex, "^QUIT$", REG_EXTENDED | REG_ICASE);
    if (regexec(&regex, command, 0, NULL, 0) != 0) {
        snprintf(reply, len, "500\r\n");
    } else {
        smtp_free_session(session);
        snprintf(reply, len, "221 OK\r\n");
    }
    return -1;
}



int smtp_process_command(char* command, smtp_session_t* session, char* reply, size_t len, const char* mail_path) {
    buffer_reset(session);
    if (session->state == MAIL_DATA) {
        return smtp_read_data(command, session, reply, len, mail_path);
    }
    command = strtrim(command);
    if (strstartswith(command, "EHLO", 1) || strstartswith(command, "HELO", 1)) {
        return smtp_ehlo(command, session, reply, len);
    } else if (strstartswith(command, "MAIL", 1)) {
        return smtp_mail(command, session, reply, len);
    } else if (strstartswith(command, "RCPT", 1)) {
        return smtp_rcpt(command, session, reply, len);
    } else if (strstartswith(command, "DATA", 1)) {
        return smtp_data(command, session, reply, len);
    } else if (strstartswith(command, "RSET", 1)) {
        return smtp_reset(command, session, reply, len);
    } else if (strstartswith(command, "QUIT", 1)) {
        return smtp_quit(command, session, reply, len);
    } else if (strstartswith(command, "VRFY", 1) || strstartswith(command, "VERIFY", 1)) {
        snprintf(reply, len, "502 Not implemented\r\n");
        return 0;
    }
    snprintf(reply, len, "500 Unknown command\r\n");
    return 0;
}

