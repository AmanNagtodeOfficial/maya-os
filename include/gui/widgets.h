#ifndef WIDGETS_H
#define WIDGETS_H

#include <stdint.h>
#include "gui/window.h"

// Widget types
typedef enum {
    WIDGET_BUTTON,
    WIDGET_LABEL,
    WIDGET_TEXTBOX,
    WIDGET_CHECKBOX,
    WIDGET_RADIOBUTTON,
    WIDGET_LISTBOX,
    WIDGET_SCROLLBAR,
    WIDGET_PANEL
} widget_type_t;

// Base widget structure
typedef struct widget {
    widget_type_t type;
    int x, y;
    int width, height;
    uint8_t visible;
    uint8_t enabled;
    uint32_t id;
    char *text;
    struct widget *parent;
    struct widget *next;     // Sibling
    struct widget *children; // First child
    void (*on_click)(struct widget *widget);
    void (*on_focus)(struct widget *widget);
    void (*on_blur)(struct widget *widget);
    void (*draw)(struct widget *widget, window_t *window);
} widget_t;

// Button widget
typedef struct {
    widget_t base;
    uint32_t color;
    uint8_t pressed;
} button_t;

// Label widget
typedef struct {
    widget_t base;
    uint32_t color;
    uint8_t alignment; // 0=left, 1=center, 2=right
} label_t;

// Textbox widget
typedef struct {
    widget_t base;
    char *buffer;
    int cursor_pos;
    int max_length;
    uint32_t color;
    uint8_t focused;
} textbox_t;

// Widget management functions
widget_t *widget_create(widget_type_t type, int x, int y, int width, int height);
void widget_destroy(widget_t *widget);
void widget_add_child(widget_t *parent, widget_t *child);
void widget_set_text(widget_t *widget, const char *text);
void widget_draw(widget_t *widget, window_t *window);

// Button functions
button_t *button_create(int x, int y, int width, int height, const char *text);
void button_draw(window_t *window, button_t *button);

// Label functions
label_t *label_create(int x, int y, const char *text);
void label_draw(window_t *window, label_t *label);

// Textbox functions
textbox_t *textbox_create(int x, int y, int width, int height);
void textbox_draw(window_t *window, textbox_t *textbox);

#endif
