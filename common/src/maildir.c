#include "maildir.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_s_ifmt.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <time.h>
#include <unistd.h>


int maildir_tmp(const char* maildir, const char* mailbox, char* name, size_t len) {
    char date[11] = { 0 };
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d", tm);

    snprintf(name, len, "%s_XXXXXX", date);
    char path[PATH_MAX] = { 0 };
    int offset = snprintf(path, PATH_MAX, "%s/%s", maildir, mailbox);
    if (mkdir(path, S_IRWXU) != 0 && errno != EEXIST) {
        return -1;
    }
    offset += snprintf(path + offset, PATH_MAX, "/tmp");
    if (mkdir(path, S_IRWXU) != 0 && errno != EEXIST) {
        return -1;
    }
    snprintf(path + offset, PATH_MAX, "/%s", name);
    int ret = mkstemp(path);
    if (ret != -1) {
        strncpy(name, path + offset + 1, len);
    }
    return ret;
}

int maildir_new(const char* maildir, const char* mailbox, const char* name) {
    char tmp_path[PATH_MAX] = { 0 };
    snprintf(tmp_path, PATH_MAX, "%s/%s/tmp/%s", maildir, mailbox, name);
    char new_path[PATH_MAX] = { 0 };
    int offset = snprintf(new_path, PATH_MAX, "%s/%s/new", maildir, mailbox);
    if (mkdir(new_path, S_IRWXU) != 0 && errno != EEXIST) {
        return -1;
    }
    snprintf(new_path + offset, PATH_MAX, "/%s", name);
    return rename(tmp_path, new_path);
}

