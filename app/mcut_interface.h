#ifndef __MCUT_INTERFACE_H__
#define __MCUT_INTERFACE_H__

#include <stdio.h>
#include "mcut_config.h"

void mcut_new_char_from_ISR(char c);
void mcut_new_string_from_ISR(const char *str, size_t str_size);
void mcut_process_new_input(void);

#endif /* __MCUT_INTERFACE_H__ */