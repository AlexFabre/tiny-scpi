#include <string.h>
#include <stdlib.h>

#include "../inc/circular_buffer.h"

/* GNU GCC MAX(a,b) recomendation for a safe macro
 * https://gcc.gnu.org/onlinedocs/gcc/Typeof.html */
#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

/* GNU GCC MIN(a,b) recomendation for a safe macro
 * https://gcc.gnu.org/onlinedocs/gcc/Typeof.html */
#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })

/* Space available in circular buffer */
size_t circ_buff_free_space(circular_buffer_t *buff)
{
    if ((buff->tail == buff->head) && (buff->full == true)) {
        return 0;
    }

    if (buff->tail >= buff->head) {
        return buff->max_size - (buff->tail - buff->head);
    } else {
        return (buff->head - buff->tail);
    }
}

/* Current size of circular buffer */
size_t circ_buff_current_size(circular_buffer_t *buff)
{
    if ((buff->tail == buff->head) && (buff->full == true)) {
        return buff->max_size;
    }

    if (buff->tail >= buff->head) {
        return (buff->tail - buff->head);
    } else {
        return ((buff->max_size) - (buff->head - buff->tail));
    }
}

/* Maximum capacity of circular buffer */
size_t circ_buffer_max_size(circular_buffer_t *buff)
{
    return buff->max_size;
}

/* Add single char to circular buffer */
void circ_buff_push_char(circular_buffer_t *buff, const char c)
{
    *(buff->array + buff->tail) = c;
    buff->tail = (buff->tail + 1) % buff->max_size;

    if (buff->full) {
        buff->head = (buff->head + 1) % buff->max_size;
    } else {
        if (buff->tail == buff->head) {
            buff->full = true;
        }
    }
}

/* Add string to circular buffer
 * If string size is too big, only the first chars will be copied */
void circ_buff_push_string(circular_buffer_t *buff, const char *str, size_t str_size)
{
    if (str_size >= buff->max_size) {
        memcpy(buff->array, str, buff->max_size);
        buff->head = 0;
        buff->tail = 0;
        buff->full = true;
    } else {
        memcpy(buff->array, str, str_size);
        buff->head = 0;
        buff->tail = str_size;
        buff->full = false;
    }
}

/* Read last char in circular buffer */
char circ_buff_read_char(circular_buffer_t *buff, int pos)
{
    long cursor;

    if (pos >= 0) {
        /* count up */
        cursor = (buff->head + pos) % buff->max_size;
    } else {
        /* count down */
        cursor = (buff->tail + pos) % buff->max_size;
    }

    return *(buff->array + cursor);
}

/* Read certain size of the circular buffer */
void circ_buff_read_all(circular_buffer_t *buff, char *buffer_out, size_t read_size)
{
    size_t len;
    if (buff->tail > buff->head) {
        len = MIN(read_size, (buff->tail - buff->head));
        memcpy(buffer_out, buff->array + buff->head, len);
    } else {
        size_t top = buff->max_size - buff->head;
        len = MIN(read_size, top);
        memcpy(buffer_out, buff->array + buff->head, len);

        len = MIN((read_size - top), buff->tail);
        memcpy(buffer_out + top, buff->array, len);
    }
}

/* Clear circular buffer */
void circ_buff_remove_n_chars(circular_buffer_t *buff, int pos)
{
    if (circ_buff_current_size(buff) >= (unsigned)abs(pos)) {
        if (pos < 0) {
            buff->tail = (buff->tail + pos) % buff->max_size;
        } else {
            buff->head = (buff->head + pos) % buff->max_size;
        }
    }
}

/* Clear circular buffer */
void circ_buff_flush(circular_buffer_t *buff)
{
    buff->full = false;
    buff->head = buff->tail;
}
