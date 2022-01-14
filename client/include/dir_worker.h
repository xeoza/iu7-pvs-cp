#ifndef DIR_WORKER_H
#define DIR_WORKER_H

typedef struct Mail_files {
    char** files;
    int count;
} mail_files_t;

mail_files_t* check_directory(const char* dir_path);

int get_files_count(const char* dir_path);

int get_files_names(const char* dir_path, char** files_names, int files_count);

int clear_mail_files(mail_files_t* files);

#endif
