#include "gui/desktop.h"

void desktop_draw(Window* window) {
    // Draw the desktop background
    // This could involve drawing a solid color, a gradient, or an image

    // Draw any icons or widgets on the desktop
    // This would typically involve iterating through a list of desktop items

    // Redraw the taskbar on top of the desktop
    taskbar_draw(&taskbar);
}

// Other desktop-related functions (e.g., handling icon clicks) would go here

