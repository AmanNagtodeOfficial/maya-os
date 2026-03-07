/**
 * Maya OS Security Model
 * Author: AmanNagtodeOfficial
 */
#ifndef KERNEL_SECURITY_H
#define KERNEL_SECURITY_H

#include <stdint.h>
#include <stdbool.h>

/* Capability bits */
#define SEC_CAP_NET      (1u << 0)  /* Network access          */
#define SEC_CAP_DISK     (1u << 1)  /* Raw disk access         */
#define SEC_CAP_GUI      (1u << 2)  /* GUI / framebuffer       */
#define SEC_CAP_PROC     (1u << 3)  /* Process management      */
#define SEC_CAP_SYS      (1u << 4)  /* System calls            */
#define SEC_CAP_AUDIO    (1u << 5)  /* Audio subsystem         */
#define SEC_CAP_ALL      (0xFFFFFFFFu)

bool     sec_init(void);
bool     sec_grant_cap(uint32_t pid, uint32_t caps);
bool     sec_check_cap(uint32_t pid, uint32_t cap);
bool     sec_revoke_cap(uint32_t pid, uint32_t caps);
uint32_t sec_get_caps(uint32_t pid);
bool     sec_is_initialized(void);

#endif /* KERNEL_SECURITY_H */
