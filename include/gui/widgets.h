
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
                                                                    struct widget *next;
                                                                        void (*on_click)(struct widget *widget);
                                                                            void (*on_focus)(struct widget *widget);
                                                                                void (*on_blur)(struct widget *widget);
                                                                                    void (*draw)(struct widget *widget, window_t *window);
                                                                                    } widget_t;

                                                                                    // Button widget
                                                                                    typedef struct {
                                                                                        widget_t base;
                                                                                            uint8_t color;
                                                                                                uint8_t pressed;
                                                                                                    void (*on_click)(void);
                                                                                                    } button_t;

                                                                                                    // Label widget
                                                                                                    typedef struct {
                                                                                                        widget_t base;
                                                                                                            uint8_t color;
                                                                                                                uint8_t alignment; // 0=left, 1=center, 2=right
                                                                                                                } label_t;

                                                                                                                // Textbox widget
                                                                                                                typedef struct {
                                                                                                                    widget_t base;
                                                                                                                        char *buffer;
                                                                                                                            int cursor_pos;
                                                                                                                                int max_length;
                                                                                                                                    uint8_t color;
                                                                                                                                        uint8_t focused;
                                                                                                                                        } textbox_t;

                                                                                                                                        // Checkbox widget
                                                                                                                                        typedef struct {
                                                                                                                                            widget_t base;
                                                                                                                                                uint8_t checked;
                                                                                                                                                    uint8_t color;
                                                                                                                                                        void (*on_change)(int checked);
                                                                                                                                                        } checkbox_t;

                                                                                                                                                        // Radio button widget
                                                                                                                                                        typedef struct {
                                                                                                                                                            widget_t base;
                                                                                                                                                                uint8_t checked;
                                                                                                                                                                    uint8_t color;
                                                                                                                                                                        uint32_t group_id;
                                                                                                                                                                            void (*on_change)(int checked);
                                                                                                                                                                            } radiobutton_t;

                                                                                                                                                                            // Listbox widget
                                                                                                                                                                            typedef struct listbox_item {
                                                                                                                                                                                char *text;
                                                                                                                                                                                    void *data;
                                                                                                                                                                                        struct listbox_item *next;
                                                                                                                                                                                        } listbox_item_t;

                                                                                                                                                                                        typedef struct {
                                                                                                                                                                                            widget_t base;
                                                                                                                                                                                                listbox_item_t *items;
                                                                                                                                                                                                    int selected_index;
                                                                                                                                                                                                        int item_count;
                                                                                                                                                                                                            int visible_items;
                                                                                                                                                                                                                int scroll_pos;
                                                                                                                                                                                                                    uint8_t color;
                                                                                                                                                                                                                        void (*on_select)(int index, void *data);
                                                                                                                                                                                                                        } listbox_t;

                                                                                                                                                                                                                        // Scrollbar widget
                                                                                                                                                                                                                        typedef struct {
                                                                                                                                                                                                                            widget_t base;
                                                                                                                                                                                                                                int min_value;
                                                                                                                                                                                                                                    int max_value;
                                                                                                                                                                                                                                        int current_value;
                                                                                                                                                                                                                                            int page_size;
                                                                                                                                                                                                                                                uint8_t orientation; // 0=horizontal, 1=vertical
                                                                                                                                                                                                                                                    uint8_t color;
                                                                                                                                                                                                                                                        void (*on_scroll)(int value);
                                                                                                                                                                                                                                                        } scrollbar_t;

                                                                                                                                                                                                                                                        // Panel widget (container)
                                                                                                                                                                                                                                                        typedef struct {
                                                                                                                                                                                                                                                            widget_t base;
                                                                                                                                                                                                                                                                widget_t *children;
                                                                                                                                                                                                                                                                    uint8_t background_color;
                                                                                                                                                                                                                                                                        uint8_t border_color;
                                                                                                                                                                                                                                                                            uint8_t has_border;
                                                                                                                                                                                                                                                                            } panel_t;

                                                                                                                                                                                                                                                                            // Widget management functions
                                                                                                                                                                                                                                                                            widget_t *widget_create(widget_type_t type, int x, int y, int width, int height);
                                                                                                                                                                                                                                                                            void widget_destroy(widget_t *widget);
                                                                                                                                                                                                                                                                            void widget_add_child(widget_t *parent, widget_t *child);
                                                                                                                                                                                                                                                                            void widget_remove_child(widget_t *parent, widget_t *child);
                                                                                                                                                                                                                                                                            void widget_set_position(widget_t *widget, int x, int y);
                                                                                                                                                                                                                                                                            void widget_set_size(widget_t *widget, int width, int height);
                                                                                                                                                                                                                                                                            void widget_show(widget_t *widget);
                                                                                                                                                                                                                                                                            void widget_hide(widget_t *widget);
                                                                                                                                                                                                                                                                            void widget_enable(widget_t *widget);
                                                                                                                                                                                                                                                                            void widget_disable(widget_t *widget);
                                                                                                                                                                                                                                                                            void widget_set_text(widget_t *widget, const char *text);
                                                                                                                                                                                                                                                                            void widget_draw(widget_t *widget, window_t *window);
                                                                                                                                                                                                                                                                            void widget_handle_click(widget_t *widget, int x, int y);
                                                                                                                                                                                                                                                                            void widget_handle_key(widget_t *widget, char key);
                                                                                                                                                                                                                                                                            widget_t *widget_find_at_pos(widget_t *parent, int x, int y);

                                                                                                                                                                                                                                                                            // Button functions
                                                                                                                                                                                                                                                                            button_t *button_create(int x, int y, int width, int height, const char *text);
                                                                                                                                                                                                                                                                            void button_destroy(button_t *button);
                                                                                                                                                                                                                                                                            void button_set_text(button_t *button, const char *text);
                                                                                                                                                                                                                                                                            void button_set_callback(button_t *button, void (*callback)(void));
                                                                                                                                                                                                                                                                            void button_draw(window_t *window, button_t *button);
                                                                                                                                                                                                                                                                            uint8_t button_handle_click(button_t *button, int x, int y);

                                                                                                                                                                                                                                                                            // Label functions
                                                                                                                                                                                                                                                                            label_t *label_create(int x, int y, const char *text);
                                                                                                                                                                                                                                                                            void label_destroy(label_t *label);
                                                                                                                                                                                                                                                                            void label_set_text(label_t *label, const char *text);
                                                                                                                                                                                                                                                                            void label_set_alignment(label_t *label, uint8_t alignment);
                                                                                                                                                                                                                                                                            void label_draw(window_t *window, label_t *label);

                                                                                                                                                                                                                                                                            // Textbox functions
                                                                                                                                                                                                                                                                            textbox_t *textbox_create(int x, int y, int width, int height);
                                                                                                                                                                                                                                                                            void textbox_destroy(textbox_t *textbox);
                                                                                                                                                                                                                                                                            void textbox_set_text(textbox_t *textbox, const char *text);
                                                                                                                                                                                                                                                                            char *textbox_get_text(textbox_t *textbox);
                                                                                                                                                                                                                                                                            void textbox_clear(textbox_t *textbox);
                                                                                                                                                                                                                                                                            void textbox_set_max_length(textbox_t *textbox, int max_length);
                                                                                                                                                                                                                                                                            void textbox_draw(window_t *window, textbox_t *textbox);
                                                                                                                                                                                                                                                                            void textbox_handle_key(textbox_t *textbox, char key);

                                                                                                                                                                                                                                                                            // Checkbox functions
                                                                                                                                                                                                                                                                            checkbox_t *checkbox_create(int x, int y, const char *text);
                                                                                                                                                                                                                                                                            void checkbox_destroy(checkbox_t *checkbox);
                                                                                                                                                                                                                                                                            void checkbox_set_checked(checkbox_t *checkbox, uint8_t checked);
                                                                                                                                                                                                                                                                            uint8_t checkbox_is_checked(checkbox_t *checkbox);
                                                                                                                                                                                                                                                                            void checkbox_draw(window_t *window, checkbox_t *checkbox);
                                                                                                                                                                                                                                                                            uint8_t checkbox_handle_click(checkbox_t *checkbox, int x, int y);

                                                                                                                                                                                                                                                                            // Radio button functions
                                                                                                                                                                                                                                                                            radiobutton_t *radiobutton_create(int x, int y, const char *text, uint32_t group_id);
                                                                                                                                                                                                                                                                            void radiobutton_destroy(radiobutton_t *radiobutton);
                                                                                                                                                                                                                                                                            void radiobutton_set_checked(radiobutton_t *radiobutton, uint8_t checked);
                                                                                                                                                                                                                                                                            uint8_t radiobutton_is_checked(radiobutton_t *radiobutton);
                                                                                                                                                                                                                                                                            void radiobutton_draw(window_t *window, radiobutton_t *radiobutton);
                                                                                                                                                                                                                                                                            uint8_t radiobutton_handle_click(radiobutton_t *radiobutton, int x, int y);

                                                                                                                                                                                                                                                                            // Listbox functions
                                                                                                                                                                                                                                                                            listbox_t *listbox_create(int x, int y, int width, int height);
                                                                                                                                                                                                                                                                            void listbox_destroy(listbox_t *listbox);
                                                                                                                                                                                                                                                                            void listbox_add_item(listbox_t *listbox, const char *text, void *data);
                                                                                                                                                                                                                                                                            void listbox_remove_item(listbox_t *listbox, int index);
                                                                                                                                                                                                                                                                            void listbox_clear(listbox_t *listbox);
                                                                                                                                                                                                                                                                            void listbox_set_selected(listbox_t *listbox, int index);
                                                                                                                                                                                                                                                                            int listbox_get_selected(listbox_t *listbox);
                                                                                                                                                                                                                                                                            void *listbox_get_selected_data(listbox_t *listbox);
                                                                                                                                                                                                                                                                            void listbox_draw(window_t *window, listbox_t *listbox);
                                                                                                                                                                                                                                                                            uint8_t listbox_handle_click(listbox_t *listbox, int x, int y);

                                                                                                                                                                                                                                                                            // Scrollbar functions
                                                                                                                                                                                                                                                                            scrollbar_t *scrollbar_create(int x, int y, int width, int height, uint8_t orientation);
                                                                                                                                                                                                                                                                            void scrollbar_destroy(scrollbar_t *scrollbar);
                                                                                                                                                                                                                                                                            void scrollbar_set_range(scrollbar_t *scrollbar, int min_value, int max_value);
                                                                                                                                                                                                                                                                            void scrollbar_set_value(scrollbar_t *scrollbar, int value);
                                                                                                                                                                                                                                                                            int scrollbar_get_value(scrollbar_t *scrollbar);
                                                                                                                                                                                                                                                                            void scrollbar_set_page_size(scrollbar_t *scrollbar, int page_size);
                                                                                                                                                                                                                                                                            void scrollbar_draw(window_t *window, scrollbar_t *scrollbar);
                                                                                                                                                                                                                                                                            uint8_t scrollbar_handle_click(scrollbar_t *scrollbar, int x, int y);

                                                                                                                                                                                                                                                                            // Panel functions
                                                                                                                                                                                                                                                                            panel_t *panel_create(int x, int y, int width, int height);
                                                                                                                                                                                                                                                                            void panel_destroy(panel_t *panel);
                                                                                                                                                                                                                                                                            void panel_set_background_color(panel_t *panel, uint8_t color);
                                                                                                                                                                                                                                                                            void panel_set_border(panel_t *panel, uint8_t has_border, uint8_t border_color);
                                                                                                                                                                                                                                                                            void panel_draw(window_t *window, panel_t *panel);

                                                                                                                                                                                                                                                                            #endif
