#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

/**
 * Функция помещает значение error_text в stderr и вызывает немедленное окончание работы программы.
 * @param error_text
 */
void exit_on_error(const char* error_text);

/**
 * Функция помещает значение warning_text stderr.
 * @param error_text
 */
void warn_on_error(const char* warning_text);

#endif
