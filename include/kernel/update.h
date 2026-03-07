/**
 * Maya OS Update System
 * Author: AmanNagtodeOfficial
 */
#ifndef KERNEL_UPDATE_H
#define KERNEL_UPDATE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} update_version_t;

typedef enum {
    UPDATE_STATUS_IDLE       = 0,
    UPDATE_STATUS_CHECKING,
    UPDATE_STATUS_AVAILABLE,
    UPDATE_STATUS_DOWNLOADING,
    UPDATE_STATUS_APPLYING,
    UPDATE_STATUS_SUCCESS,
    UPDATE_STATUS_ERROR,
} update_status_t;

bool           update_init(void);
update_status_t update_get_status(void);
bool           update_check(void);
bool           update_apply(void);
update_version_t update_get_current_version(void);
update_version_t update_get_remote_version(void);

#endif /* KERNEL_UPDATE_H */
