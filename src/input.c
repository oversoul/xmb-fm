#include "input.h"
#include <string.h>

void show_input(Input *input) {
    input->is_visible = true; //
}

void hide_input(Input *input) {
    memset(input->buffer, 0, INPUT_BUFFER_LEN);
    input->is_visible = false;
    input->position = 0;
}

void set_buffer_input(Input *input, const char *text) {
    strcpy(input->buffer, text); //
    input->position = strlen(text);
}

void move_cursor_left(Input *input) {
    if (input->position > 0) {
        input->position--;
    }
}

void move_cursor_right(Input *input) {
    if (input->position < strlen(input->buffer)) {
        input->position++;
    }
}

void pop_from_input(Input *input) {
    if (input->position <= 0)
        return;

    size_t len = strlen(input->buffer);
    if (len == 0)
        return;

    int char_start = input->position - 1;
    while (char_start > 0 && (input->buffer[char_start] & 0xC0) == 0x80) {
        char_start--;
    }

    int char_length = input->position - char_start;

    for (int i = char_start; i <= len - char_length; i++) {
        input->buffer[i] = input->buffer[i + char_length];
    }

    input->position = char_start;
}

void append_to_input(Input *input, unsigned int codepoint) {
    char utf8_char[5] = {0};
    int utf8_len = 0;

    if (codepoint < 0x80) {
        // ASCII
        utf8_char[0] = (char)codepoint;
        utf8_len = 1;
    } else if (codepoint < 0x800) {
        // 2-byte UTF-8
        utf8_char[0] = 0xC0 | (codepoint >> 6);
        utf8_char[1] = 0x80 | (codepoint & 0x3F);
        utf8_len = 2;
    } else if (codepoint < 0x10000) {
        // 3-byte UTF-8
        utf8_char[0] = 0xE0 | (codepoint >> 12);
        utf8_char[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        utf8_char[2] = 0x80 | (codepoint & 0x3F);
        utf8_len = 3;
    } else if (codepoint < 0x110000) {
        // 4-byte UTF-8
        utf8_char[0] = 0xF0 | (codepoint >> 18);
        utf8_char[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        utf8_char[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        utf8_char[3] = 0x80 | (codepoint & 0x3F);
        utf8_len = 4;
    } else {
        // Invalid Unicode code point
        return;
    }

    size_t len = strlen(input->buffer);

    if (len < INPUT_BUFFER_LEN - 1) {
        for (int i = len; i >= input->position; i--) {
            input->buffer[i + 1] = input->buffer[i];
        }
        input->buffer[input->position] = (unsigned char)codepoint;
        input->position++;
    }
}
