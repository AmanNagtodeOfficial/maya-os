/**
 * Maya OS Shell Library
 * Updated: 2026-03-07 21:30:00 UTC
 * Author: AmanNagtodeOfficial
 */

#include "apps/shell.h"
#include "fs/vfs.h"
#include "libc/string.h"
#include "libc/stdio.h"
#include "kernel/memory.h"

#define MAX_HISTORY 20
#define MAX_COMMAND_LEN 256

static char history[MAX_HISTORY][MAX_COMMAND_LEN];
static int history_count = 0;
static int history_pos = 0;
static shell_output_t shell_out = NULL;

void shell_init(shell_output_t output_func) {
    shell_out = output_func;
    memset(history, 0, sizeof(history));
    history_count = 0;
    history_pos = 0;
}

static void shell_cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;
    shell_out("Available commands: help, clear, ls, cat, echo, exit");
}

static void shell_cmd_ls(int argc, char** argv) {
    char* path = (argc > 1) ? argv[1] : "/";
    vfs_node_t* node = vfs_finddir(vfs_root, path); // Simplified finddir
    if (!node) {
        shell_out("Directory not found.");
        return;
    }
    
    struct vfs_dirent* entry;
    int i = 0;
    while ((entry = vfs_readdir(node, i++))) {
        shell_out(entry->name);
    }
}

static void shell_cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        shell_out(argv[i]);
        if (i < argc - 1) shell_out(" ");
    }
    shell_out("\n");
}

typedef void (*shell_cmd_handler_t)(int argc, char** argv);

typedef struct {
    char* name;
    shell_cmd_handler_t handler;
} shell_command_t;

static shell_command_t commands[] = {
    {"help", shell_cmd_help},
    {"ls", shell_cmd_ls},
    {"echo", shell_cmd_echo},
    {NULL, NULL}
};

void shell_execute(const char* command) {
    if (!command || strlen(command) == 0) return;

    // Add to history
    strncpy(history[history_count % MAX_HISTORY], command, MAX_COMMAND_LEN - 1);
    history_count++;

    // Tokenize
    char buf[MAX_COMMAND_LEN];
    strcpy(buf, command);
    char* argv[16];
    int argc = 0;
    char* token = strtok(buf, " ");
    while (token && argc < 16) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return;

    // Dispatch
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].handler(argc, argv);
            return;
        }
    }

    shell_out("Unknown command. Type 'help'.");
}

void shell_autocomplete(const char* part, char* result) {
    if (!part || !result) return;
    // Basic command autocomplete
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strncmp(part, commands[i].name, strlen(part)) == 0) {
            strcpy(result, commands[i].name);
            return;
        }
    }
}

const char* shell_get_history(int index) {
    if (index < 0 || index >= MAX_HISTORY || index >= history_count) return NULL;
    int actual_idx = (history_count - 1 - index) % MAX_HISTORY;
    if (actual_idx < 0) actual_idx += MAX_HISTORY;
    return history[actual_idx];
}

int shell_get_history_count(void) {
    return (history_count > MAX_HISTORY) ? MAX_HISTORY : history_count;
}
