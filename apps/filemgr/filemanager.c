#include "apps/filemanager.h"
#include "gui/window.h"
#include "gui/widgets.h"
#include "fs/vfs.h"

typedef struct {
    window_t *window;
    char current_path[256];
    file_entry_t *files;
    int file_count;
    int selected_index;
} file_manager_t;

file_manager_t *file_manager_create(void);
void file_manager_refresh(file_manager_t *fm);
void file_manager_navigate(file_manager_t *fm, const char *path);
void file_manager_run(file_manager_t *fm);