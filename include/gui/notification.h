#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NOTIFICATIONS 8
#define NOTIFICATION_DURATION 5000 // 5 seconds

typedef struct {
    char title[64];
    char message[128];
    uint32_t start_time;
    bool active;
} notification_t;

void notification_init(void);
void notification_push(const char* title, const char* message);
void notification_render(void);
void notification_update(void);

#endif
