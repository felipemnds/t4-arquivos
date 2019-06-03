#ifndef FUNCOES_MONITOR_H
#define FUNCOES_MONITOR_H

#include <stdio.h>

void scan_quote_string(char *str);

void trim(char *str);

void binarioNaTela1(FILE *ponteiroArquivoBinario);

void MS_sort(void *vector, unsigned long n, size_t memsize, int (*fcmp)(const void *, const void *));

#endif
