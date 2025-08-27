
#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <stdint.h> // Include standard integer types

#define LAUNCHER_WIDTH 64 // Width of the launcher in pixels
#define MAX_LAUNCHER_ITEMS 16 // Maximum number of items the launcher can hold
#define LAUNCHER_ITEM_SIZE 48 // Size of each launcher item (icon) in pixels
#define LAUNCHER_PADDING 8 // Padding around launcher items in pixels

// Structure to represent a single item in the launcher
typedef struct {
    char *name; // Display name of the item
    char *command; // Command to execute when the item is clicked
    uint8_t *icon_data; // Pixel data for the item's icon
    uint8_t pinned; // Flag indicating if the item is pinned to the launcher
    uint8_t running; // Flag indicating if the application is currently running
    uint8_t highlighted; // Flag indicating if the item is currently highlighted (e.g., on hover)
} launcher_item_t;

// Structure to represent the launcher itself
typedef struct {
    launcher_item_t items[MAX_LAUNCHER_ITEMS]; // Array of launcher items
    int item_count; // Current number of items in the launcher
    int scroll_offset; // Vertical scroll offset for the launcher
    uint8_t auto_hide; // Flag indicating if the launcher should auto-hide
    uint8_t visible; // Flag indicating if the launcher is currently visible
} launcher_t;

// Launcher functions
void launcher_init(void); // Initializes the launcher
void launcher_render(void); // Renders the launcher on the screen
void launcher_add_item(const char *name, const char *command, uint8_t *icon); // Adds an item to the launcher
void launcher_remove_item(int index); // Removes an item from the launcher at the specified index
void launcher_handle_click(int x, int y); // Handles a mouse click event on the launcher
void launcher_set_auto_hide(uint8_t enabled); // Sets the auto-hide behavior of the launcher

#endif // LAUNCHER_H

                                            #endif
