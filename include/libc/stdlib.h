#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);

int abs(int j);
long labs(long j);
long long llabs(long long j);

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

#endif
