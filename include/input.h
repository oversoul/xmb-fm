#pragma once

#define INPUT_BUFFER_LEN 60

typedef struct {
    int position;
    bool is_visible;
    char buffer[INPUT_BUFFER_LEN];
} Input;

void show_input(Input *input);
void hide_input(Input *input);
void pop_from_input(Input *input);
void set_buffer_input(Input *input, const char *text);
void append_to_input(Input *input, unsigned int codepoint);

void move_cursor_left(Input *input);
void move_cursor_right(Input *input);
