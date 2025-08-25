#ifndef WIDGETS_H
#define WIDGETS_H

#include <stdint.h>
#include "window.h"

// Button widget
typedef struct {
    int x, y;
    int width, height;
    char *text;
    uint8_t color;
    void (*on_click)(void);
} button_t;

// Label widget
typedef struct {
    int x, y;
    int width, height;
    char *text;
    uint8_t color;
} label_t;

// Textbox widget
typedef struct {
    int x, y;
    int width, height;
    char *buffer;
    uint8_t color;
    int cursor_pos;
} textbox_t;

// Button functions
button_t *button_create(int x, int y, int width, int height, const char *text);
void button_destroy(button_t *button);
void button_draw(window_t *window, button_t *button);
uint8_t button_handle_click(button_t *button, int x, int y);

// Label functions
label_t *label_create(int x, int y, const char *text);
void label_destroy(label_t *label);
void label_draw(window_t *window, label_t *label);
void label_set_text(label_t *label, const char *text);

// Textbox functions
textbox_t *textbox_create(int x, int y, int width, int height);
void textbox_destroy(textbox_t *textbox);
void textbox_draw(window_t *window, textbox_t *textbox);
void textbox_handle_key(textbox_t *textbox, char key);

#endif

