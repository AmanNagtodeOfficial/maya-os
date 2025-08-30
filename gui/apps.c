#include "gui/apps.h"
#include "gui/desktop.h"
#include "gui/graphics.h"
#include "gui/input.h"
#include "kernel/memory.h"
#include "libc/string.h"
#include "libc/stdio.h"
#include "libc/stdlib.h"

// Application interface table
static maya_app_interface_t app