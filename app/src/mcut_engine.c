#include <string.h>

#include "../inc/circular_buffer.h"
#include "../inc/mcut_engine.h"
#include "../mcut_config.h"
#include "../mcut_interface.h"

static char allocated_rx_buffer[MCUT_BUFFER_SIZE] = {};
static char input[MCUT_BUFFER_SIZE+1] = {};
static char previous_cmd[MCUT_BUFFER_SIZE] = {};

static bool new_input_received = false;

static circular_buffer_t rx_buff = {
    .array = allocated_rx_buffer,
    .head = 0,
    .tail = 0,
    .max_size = MCUT_BUFFER_SIZE,
    .full = false
};

void mcut_new_char_from_ISR(char c) {
    circ_buff_push_char(&rx_buff, c);
    new_input_received = true;
}

void mcut_new_string_from_ISR(const char *str, size_t str_size) {
    circ_buff_push_string(&rx_buff, str, str_size);
    new_input_received = true;
}

void mcut_process_new_input(void) {
    if(new_input_received == false){
        return;
    }

    size_t len = circ_buff_current_size(&rx_buff);
    char c = circ_buff_read_char(&rx_buff, -1);

    while ((c == '\0') && len)
    {
        circ_buff_remove_n_chars(&rx_buff, -1);
        c = circ_buff_read_char(&rx_buff, -1);
        len = circ_buff_current_size(&rx_buff);
    }

    switch(c) {
        case '\t':
            mcut_infoPrint("Tab detected !\n");
            break;
        case '\b':
            circ_buff_remove_n_chars(&rx_buff, -2);
            break;
        case CHAR_END_OF_CMD:
        case '\n':
        case '\r':
            if (len > MINIMUM_CMD_SIZE) {
                memset(input, '\0', MCUT_BUFFER_SIZE);
                circ_buff_read_all(&rx_buff, input, MCUT_BUFFER_SIZE);
                resolve_command(input, strlen(input));
            }
            strncpy(previous_cmd, input, strlen(input));
            circ_buff_flush(&rx_buff);
            break;
        default:
            break;
    }
    new_input_received = false;
}
