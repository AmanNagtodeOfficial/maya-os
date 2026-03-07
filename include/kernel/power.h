/**
 * Maya OS Power Management
 * Author: AmanNagtodeOfficial
 */
#ifndef KERNEL_POWER_H
#define KERNEL_POWER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    POWER_STATE_S0  = 0, /* Running   */
    POWER_STATE_S1  = 1, /* Standby   */
    POWER_STATE_S3  = 3, /* Sleep     */
    POWER_STATE_S4  = 4, /* Hibernate */
    POWER_STATE_S5  = 5, /* Soft Off  */
} power_state_t;

bool power_init(void);
void power_shutdown(void);
void power_reboot(void);
void power_sleep(void);
void power_hibernate(void);
power_state_t power_get_state(void);

#endif /* KERNEL_POWER_H */
