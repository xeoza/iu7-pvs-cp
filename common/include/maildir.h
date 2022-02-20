#ifndef MAILDIR_H
#define MAILDIR_H

#include <stddef.h>

#define MAILDIR_NAME_LEN 18

int maildir_tmp(const char* maildir, const char* mailbox, char* name, size_t len);

int maildir_new(const char* maildir, const char* mailbox, const char* name);

#endif  // MAILDIR_H

