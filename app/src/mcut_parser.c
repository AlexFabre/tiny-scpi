#include "../inc/mcut_engine.h"
#include "../mcut_config.h"
#include "../mcut_interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables */
const char *ok = "OK\n";
const char *error = "ERROR\n";
const char *error_not_found = "ERROR unknown cmd\n";
const char *error_needs_params = "ERROR cmd needs param\n";
const char *error_read_only = "ERROR read only cmd\n";
const char *error_write_only = "ERROR write only cmd\n";
const char *error_incomplete_cmd = "ERROR incomplete cmd\n";

/* sub commands */
command_t available_commands[];
static bool cli_initialized = false;

/* Commands callback functions */
void get_commands_list_str(command_t *commands_list, char *commands_list_str);
void entry_menu_callback(bool query, char *params, char *answer);
void iee488_idn_callback(bool query, char *params, char *answer);
void iee488_rst_callback(bool query, char *params, char *answer);

/* Help callback functions */
HELPER_CALLBACK(cmd, available_commands)

/* Other needed static functions */
static uint8_t _process_sub_commands_number(command_t *commands);

command_t available_commands[] = {
    {
        .pattern = "HELP",
        .callback = cmd_help_callback,
        .sub_commands = NULL,
        .needs_params = false,
    },
    {
        .pattern = "*IDN",
        .callback = iee488_idn_callback,
        .sub_commands = NULL,
        .needs_params = false,
    },
    {
        .pattern = "*RST",
        .callback = iee488_rst_callback,
        .sub_commands = NULL,
        .needs_params = false,
    },
    {
        .pattern = "HW",
        .callback = entry_menu_callback,
        .sub_commands = NULL,
        .needs_params = false,
    },
    {
        .pattern = "DOCK",
        .callback = entry_menu_callback,
        .sub_commands = NULL,
        .needs_params = false,
    },
    CMD_LIST_END
};

void get_commands_list_str(command_t *commands_list, char *commands_list_str)
{
    uint8_t idx = 0;

    while (commands_list[idx].pattern != NULL) {
        if (strcmp(commands_list[idx].pattern, "HELP") != 0) {
            strcat(commands_list_str, commands_list[idx].pattern);
            strcat(commands_list_str, "   ");
        }
        idx++;
    }
    // At the end of the while loop, the last character is ',', we need to delete it
    commands_list_str[strlen(commands_list_str) - 3] = '\0';
}

void entry_menu_callback(bool query, char *params, char *answer)
{
    (void) query;
    (void) params;
    strcpy(answer, error_incomplete_cmd);
}

void iee488_idn_callback(bool query, char *params, char *answer)
{
    (void) params;
    if (query) {
        answer[0] = 0;
        snprintf(answer, MCUT_RESPONSE_SIZE, "%s\n%s", "25569874562150", ok);
    } else {
        strcpy(answer, error_read_only);
    }
}

void iee488_rst_callback(bool query, char *params, char *answer)
{
    (void) params;
    if (query) {
        strcpy(answer, error_write_only);
    } else {
        /* Normally it's the resolve_command(..) function that takes care of sending the command response via uart.
         * However, since we have to reset the MCU here, the resolve_command(..) function will not have time
         * to send the response. So we decided exceptionally to send the response here before proceeding with the reset. */
        strcpy(answer, ok);
        mcut_respond("%s", answer);
    }
}

static uint8_t _process_sub_commands_number(command_t *commands)
{
    // Compute each command's sub-commands number , needed in the parsing function
    // Warning recursive function
    uint8_t idx = 0;
    command_t *elem = &commands[idx];
    while (elem->pattern != NULL) {
        if (elem->sub_commands != NULL) {
            elem->sub_cmd_number = _process_sub_commands_number(elem->sub_commands);
        } else {
            elem->sub_cmd_number = 0;
        }
        elem = &commands[++idx];
    }
    return idx;
}

static void cli_commands_init(void)
{
    _process_sub_commands_number(available_commands);
    cli_initialized = true;
}

void resolve_command(char *command, uint8_t command_size)
{
    /* /!\ Important info
     * The CLI commands are based on IEEE 488.2 and SCPI-99 (https://www.ivifoundation.org/docs/scpi-99.pdf)
     * Commands are grouped into a hierarchy or "tree" structure
     * Specific sub-commands within the hierarchy are nested with a colon (:) character
     * Commands are always sent in uppercase
     * Parameters are separated from commands with a space ( ) character and separate between them by a comma (,) character
     * Floats use dot (.) character as decimal separator
     * Parameters are queried by adding a question mark (?) to the command
     */

    int idx;
    char *strtok_rest = NULL;
    char cmd_parsing_buffer[MCUT_BUFFER_SIZE]; // buffer to avoid working on incoming command str in strtok_r
    char answer[MCUT_BUFFER_SIZE]; // answer from command callback
    bool query;
    bool parsing_done = false;
    command_t *context = &available_commands[0]; // Context set at root of available_commands
    uint8_t same_level_cmd_number = sizeof(available_commands) / sizeof(available_commands[0]);

    if (cli_initialized == false) {
        cli_commands_init();
    }

    if (command == NULL) {
        return;
    }

    if (command_size == 0) {
        return;
    }

    char *token = strtok_r(command, STRTOK_QUERY, &strtok_rest);
    if (token != NULL) {
        query = true;
        strncpy(cmd_parsing_buffer, token, command_size);
    } else {
        query = false;
        strncpy(cmd_parsing_buffer, command, command_size);
    }

    /* A command has a form similar to the following one: "HW:MODEM:INIT 1"
     * In the following lines, we will use strtok_r(..) with the separator ":" to get the different
     * parts of the command. Read https://linux.die.net/man/3/strtok_r for more details about strtok_r(..)*/
    token = strtok_r(cmd_parsing_buffer, STRTOK_CMD_SEPARATORS, &strtok_rest);
    while (token != NULL) {
        for (idx = 0; idx < same_level_cmd_number; idx++) {
            if (context[idx].pattern != NULL) {
                if (strcmp(context[idx].pattern, token) == 0) { // Match detected!
                    if (context[idx].sub_cmd_number == 0) { // the command has no child
                        if (query) { // the command ended with '?'
                            context[idx].callback(query, "", answer);
                            mcut_respond("%s", answer);
                        } else {
                            // Check if there is something left in the command
                            token = strtok_r(NULL, "", &strtok_rest); // Get next token, it must be parameters
                            if (context[idx].needs_params && token == NULL) { // The command needs params but no params received
                                mcut_respond("%s", error_needs_params);
                            } else {
                                context[idx].callback(query, token, answer); // Here token represent the command parameters
                                mcut_respond("%s", answer);
                            }
                        }
                        parsing_done = true;
                        return;
                    } else { // switching context
                        same_level_cmd_number = context[idx].sub_cmd_number; //Don't switch before retrieving command_size ;)
                        context = &context[idx].sub_commands[0];
                        idx = -1; // As we change the context, we have to reset idx variable, so the next loop iteration will start with 0 (-1+1=0)
                        token = strtok_r(NULL, STRTOK_CMD_SEPARATORS, &strtok_rest); // New context, get next token
                    }
                }
            }
        }
        if (!parsing_done) {
            mcut_respond("%s", error_not_found);
            return;
        }
        token = strtok_r(NULL, STRTOK_CMD_SEPARATORS, &strtok_rest);
    }

}
