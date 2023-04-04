#include <stdio.h>
#include <string.h>
#include "../app/mcut_config.h"
#include "../app/mcut_interface.h"

static char *test_cmd = "HELP?\r\n";

static void test_letter_by_letter(const char *string)
{
    mcut_infoPrint("Test letter by letter\n");
    for (size_t i = 0; i < strlen(string); i++) {
        mcut_new_char_from_ISR(*(string + i));
        mcut_process_new_input();
    }
    mcut_infoPrint("OK\n");
}

static void test_letter_by_letter_fast(const char *string)
{
    mcut_infoPrint("Test letter by letter fast\n");
    for (size_t i = 0; i < strlen(string); i++) {
        mcut_new_char_from_ISR(*(string + i));
    }
    mcut_process_new_input();

    mcut_infoPrint("OK\n");
}

static void test_message(const char *string)
{
    mcut_infoPrint("Test message\n");

    mcut_new_string_from_ISR(string, strlen(string));
    mcut_process_new_input();

    mcut_infoPrint("OK\n");
}

int main(int argc, char const *argv[])
{
    (void) argc;
    (void) argv;

    char input;

    mcut_infoPrint("Test start\n");

    test_letter_by_letter(test_cmd);

    test_letter_by_letter_fast(test_cmd);

    test_message(test_cmd);

    while (1) {
        scanf("%c", &input);
        mcut_new_char_from_ISR(input);
        mcut_process_new_input();
    }

    return 0;
}
