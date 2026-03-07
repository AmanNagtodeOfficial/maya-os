#ifndef APPS_SHELL_H
#define APPS_SHELL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void (*shell_output_t)(const char* text);

void shell_init(shell_output_t output_func);
void shell_execute(const char* command);
void shell_autocomplete(const char* part, char* result);
const char* shell_get_history(int index);
int shell_get_history_count(void);

#endif
