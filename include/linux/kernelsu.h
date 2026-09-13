#ifndef _LINUX_KERNELSU_H
#define _LINUX_KERNELSU_H

#include <linux/kconfig.h>
#include <linux/types.h>

#if IS_BUILTIN(CONFIG_KSU)
bool ksu_seccomp_allow_magic_reboot(int syscall_nr, unsigned long arg0, unsigned long arg1);
#else
static inline bool ksu_seccomp_allow_magic_reboot(int syscall_nr, unsigned long arg0, unsigned long arg1)
{
    return false;
}
#endif

#endif
