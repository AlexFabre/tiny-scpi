#ifndef __MCUT_ENGINE_H__
#define __MCUT_ENGINE_H__

#include <stdint.h>
#include <stdbool.h>

#define CHAR_END_OF_CMD         ';'
#define STRTOK_CMD_SEPARATORS   " :"
#define STRTOK_QUERY            "?"
#define STRTOK_ARGS_SEPARATOR   ","

#define MINIMUM_CMD_SIZE            (2)

#define MCUT_BUFFER_SIZE            (MCUT_CMD_SIZE + (MCUT_ARGUMENT_SIZE * MCUT_MAX_NB_ARGUMENTS))



/* Helper macro that generate a `name`_help_callback function which print the
 * commands available in `command_list` */
#define HELPER_CALLBACK(name, command_list) \
    static void name##_help_callback(bool query, char *params, char *answer) \
    { \
        (void) params; \
        if (query) { \
            answer[0] = 0; \
            get_commands_list_str(&(command_list[0]), answer); \
            strcat(answer, "\r\n"); \
        } else { \
            strcpy(answer, error_read_only); \
        } \
    }



typedef void(*command_callback_t)(bool query, char *params, char *answer);

typedef struct _command_t {
    const char                  *pattern;
    const command_callback_t    callback;
    const bool                  needs_params;
    uint8_t                     sub_cmd_number;
    struct _command_t           *sub_commands;
} command_t;

#define CMD_LIST_END {NULL, NULL, false , 0 , NULL}

void resolve_command(char *command, uint8_t command_size);

#endif /* __MCUT_ENGINE_H__ */