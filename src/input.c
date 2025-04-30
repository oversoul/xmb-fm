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
    size_t len = strlen(input->buffer);
    if (len > 0 && input->position > 0) {
        // Shift characters after cursor one position left
        for (int i = input->position - 1; i < len; i++) {
            input->buffer[i] = input->buffer[i + 1];
        }
        input->position--;
    }
}

void append_to_input(Input *input, unsigned int codepoint) {
    size_t len = strlen(input->buffer);

    if (len < INPUT_BUFFER_LEN - 1) {
        for (int i = len; i >= input->position; i--) {
            input->buffer[i + 1] = input->buffer[i];
        }
        input->buffer[input->position] = (unsigned char)codepoint;
        input->position++;
    }
}
