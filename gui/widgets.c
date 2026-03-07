#include "gui/widgets.h"
#include "gui/graphics.h"
#include "kernel/memory.h"
#include "libc/string.h"

static uint32_t next_widget_id = 1;

widget_t* widget_create(widget_type_t type, int x, int y, int width, int height) {
    widget_t* widget = kmalloc(sizeof(widget_t));
    if (!widget) return NULL;

    memset(widget, 0, sizeof(widget_t));
    widget->type = type;
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    widget->visible = 1;
    widget->enabled = 1;
    widget->id = next_widget_id++;

    return widget;
}

void widget_add_child(widget_t* parent, widget_t* child) {
    if (!parent || !child) return;
    child->parent = parent;
    
    if (!parent->children) {
        parent->children = child;
    } else {
        widget_t* last = parent->children;
        while (last->next) last = last->next;
        last->next = child;
    }
}

void widget_set_text(widget_t* widget, const char* text) {
    if (!widget) return;
    if (widget->text) kfree(widget->text);
    if (text) {
        widget->text = kmalloc(strlen(text) + 1);
        strcpy(widget->text, text);
    } else {
        widget->text = NULL;
    }
}

void widget_draw(widget_t* widget, window_t* window) {
    if (!widget || !window || !widget->visible) return;

    switch (widget->type) {
        case WIDGET_BUTTON:
            button_draw(window, (button_t*)widget);
            break;
        case WIDGET_LABEL:
            label_draw(window, (label_t*)widget);
            break;
        case WIDGET_TEXTBOX:
            textbox_draw(window, (textbox_t*)widget);
            break;
        default:
            break;
    }

    // Draw children
    widget_t* child = widget->children;
    while (child) {
        widget_draw(child, window);
        child = child->next;
    }
}

// Button Implementation
button_t* button_create(int x, int y, int width, int height, const char* text) {
    button_t* button = kmalloc(sizeof(button_t));
    if (!button) return NULL;

    memset(button, 0, sizeof(button_t));
    button->base.type = WIDGET_BUTTON;
    button->base.x = x;
    button->base.y = y;
    button->base.width = width;
    button->base.height = height;
    button->base.visible = 1;
    button->base.enabled = 1;
    button->base.id = next_widget_id++;
    
    if (text) {
        button->base.text = kmalloc(strlen(text) + 1);
        strcpy(button->base.text, text);
    }

    return button;
}

void button_draw(window_t* window, button_t* button) {
    if (!window || !button || !button->base.visible) return;

    // Outer border
    window_draw_rect(window, button->base.x, button->base.y, 
                    button->base.width, button->base.height, 0x000000);
    
    // Fill
    uint32_t color = button->pressed ? 0x999999 : 0xCCCCCC;
    for (int i = 1; i < button->base.height - 1; i++) {
        for (int j = 1; j < button->base.width - 1; j++) {
            window_draw_pixel(window, button->base.x + j, button->base.y + i, color);
        }
    }
    
    if (button->base.text) {
        graphics_draw_text(button->base.text, 
                          window->x + button->base.x + 5, 
                          window->y + button->base.y + (button->base.height/2) - 4, 
                          0x000000);
    }
}

// Label Implementation
label_t* label_create(int x, int y, const char* text) {
    label_t* label = kmalloc(sizeof(label_t));
    if (!label) return NULL;

    memset(label, 0, sizeof(label_t));
    label->base.type = WIDGET_LABEL;
    label->base.x = x;
    label->base.y = y;
    label->base.visible = 1;
    label->base.enabled = 1;
    label->base.id = next_widget_id++;

    if (text) {
        label->base.text = kmalloc(strlen(text) + 1);
        strcpy(label->base.text, text);
    }

    return label;
}

void label_draw(window_t* window, label_t* label) {
    if (!window || !label || !label->base.visible || !label->base.text) return;

    graphics_draw_text(label->base.text, 
                      window->x + label->base.x, 
                      window->y + label->base.y, 
                      0x000000);
}

// Textbox Implementation
textbox_t* textbox_create(int x, int y, int width, int height) {
    textbox_t* textbox = kmalloc(sizeof(textbox_t));
    if (!textbox) return NULL;

    memset(textbox, 0, sizeof(textbox_t));
    textbox->base.type = WIDGET_TEXTBOX;
    textbox->base.x = x;
    textbox->base.y = y;
    textbox->base.width = width;
    textbox->base.height = height;
    textbox->base.visible = 1;
    textbox->base.enabled = 1;
    textbox->base.id = next_widget_id++;
    
    textbox->max_length = 256;
    textbox->buffer = kmalloc(textbox->max_length);
    memset(textbox->buffer, 0, textbox->max_length);

    return textbox;
}

void textbox_draw(window_t* window, textbox_t* textbox) {
    if (!window || !textbox || !textbox->base.visible) return;

    // Outer border
    window_draw_rect(window, textbox->base.x, textbox->base.y, 
                    textbox->base.width, textbox->base.height, 0x000000);
    
    // Fill (White for textbox)
    for (int i = 1; i < textbox->base.height - 1; i++) {
        for (int j = 1; j < textbox->base.width - 1; j++) {
            window_draw_pixel(window, textbox->base.x + j, textbox->base.y + i, 0xFFFFFF);
        }
    }
    
    if (textbox->buffer) {
        graphics_draw_text(textbox->buffer, 
                          window->x + textbox->base.x + 5, 
                          window->y + textbox->base.y + (textbox->base.height/2) - 4, 
                          0x000000);
    }

    // Draw cursor if focused
    if (textbox->focused) {
        int cursor_x = textbox->base.x + 5 + (textbox->cursor_pos * 8); // Assuming 8px char width
        if (cursor_x < textbox->base.x + textbox->base.width - 5) {
            window_draw_rect(window, cursor_x, textbox->base.y + 4, 1, textbox->base.height - 8, 0x000000);
        }
    }
}
