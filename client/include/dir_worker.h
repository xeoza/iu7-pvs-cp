#ifndef DIR_WORKER_H
#define DIR_WORKER_H

typedef struct Mail_files {
    char** files;
    int count;
} mail_files_t;

/**
 * Проверка директории и загрузка писем в память
 * @param dir_path
 * @return
 */
mail_files_t* check_directory(const char* dir_path);

/**
 * Подсчет количества файлов в директории
 * @param dir_path
 * @return
 */
int get_files_count(const char* dir_path);

/**
 * Получение название файлов в директории
 * @param dir_path
 * @return
 */
int get_files_names(const char* dir_path, char** files_names, int files_count);

/**
 * Очистка писем из памяти
 * @param dir_path
 * @return
 */
int clear_mail_files(mail_files_t* files);

#endif
