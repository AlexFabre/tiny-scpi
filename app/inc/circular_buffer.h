#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    const size_t max_size;
    char *array;
    size_t head;
    size_t tail;
    bool full;
} circular_buffer_t;

size_t circ_buff_free_space(circular_buffer_t *buff);
size_t circ_buff_current_size(circular_buffer_t *buff);
size_t circ_buffer_max_size(circular_buffer_t *buff);
void circ_buff_push_char(circular_buffer_t *buff, const char c);
void circ_buff_push_string(circular_buffer_t *buff, const char *str, size_t str_size);
char circ_buff_read_char(circular_buffer_t *buff, int pos);
void circ_buff_read_all(circular_buffer_t *buff, char *buffer_out, size_t read_size);
void circ_buff_remove_n_chars(circular_buffer_t *buff, int pos);
void circ_buff_flush(circular_buffer_t *buff);

#endif /* __CIRCULAR_BUFFER_H__ */