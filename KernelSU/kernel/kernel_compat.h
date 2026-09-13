#ifndef __KSU_H_KERNEL_COMPAT
#define __KSU_H_KERNEL_COMPAT

#include <linux/fs.h>
#include <linux/task_work.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#define ksu_access_ok_read(addr, size) access_ok(addr, size)
#define ksu_access_ok_write(addr, size) access_ok(addr, size)
#else
#define ksu_access_ok_read(addr, size) access_ok(VERIFY_READ, addr, size)
#define ksu_access_ok_write(addr, size) access_ok(VERIFY_WRITE, addr, size)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
static inline long ksu_copy_from_user_nofault(void *to,
                                              const void __user *from,
                                              unsigned long count)
{
    long ret = -EFAULT;
    mm_segment_t old_fs = get_fs();

    set_fs(USER_DS);
    if (ksu_access_ok_read(from, count)) {
        pagefault_disable();
        ret = __copy_from_user_inatomic(to, from, count);
        pagefault_enable();
    }
    set_fs(old_fs);
    return ret ? -EFAULT : 0;
}

static inline long ksu_copy_to_user_nofault(void __user *to, const void *from,
                                            unsigned long count)
{
    long ret = -EFAULT;
    mm_segment_t old_fs = get_fs();

    set_fs(USER_DS);
    if (ksu_access_ok_write(to, count)) {
        pagefault_disable();
        ret = __copy_to_user_inatomic(to, from, count);
        pagefault_enable();
    }
    set_fs(old_fs);
    return ret ? -EFAULT : 0;
}

static inline long ksu_copy_to_kernel_nofault(void *to, const void *from,
                                              unsigned long count)
{
    memcpy(to, from, count);
    return 0;
}

static inline long ksu_strncpy_from_user_nofault(char *dst,
                                                 const void __user *src,
                                                 long count)
{
    long ret;
    mm_segment_t old_fs;

    if (unlikely(count <= 0))
        return 0;
    old_fs = get_fs();
    set_fs(USER_DS);
    pagefault_disable();
    ret = strncpy_from_user(dst, src, count);
    pagefault_enable();
    set_fs(old_fs);
    if (ret >= count) {
        ret = count;
        dst[ret - 1] = '\0';
    } else if (ret > 0) {
        ret++;
    }
    return ret;
}

#define copy_from_user_nofault ksu_copy_from_user_nofault
#define copy_to_user_nofault ksu_copy_to_user_nofault
#define copy_to_kernel_nofault ksu_copy_to_kernel_nofault
#define strncpy_from_user_nofault ksu_strncpy_from_user_nofault
#endif

static inline long ksu_copy_from_user_retry(void *to, const void __user *from,
                                            unsigned long count)
{
    long ret = copy_from_user_nofault(to, from, count);

    if (likely(!ret))
        return ret;
    return copy_from_user(to, from, count);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 222)
static inline ssize_t ksu_strscpy_pad(char *dest, const char *src, size_t count)
{
    ssize_t result;

    if (count == 0)
        return -E2BIG;
    result = strscpy(dest, src, count);
    if (result >= 0 && (size_t)result < count)
        memset(dest + result, 0, count - result);
    return result;
}
#define strscpy_pad ksu_strscpy_pad
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0)
#ifndef TWA_RESUME
#define TWA_RESUME true
#endif
#endif

#ifndef fallthrough
#define fallthrough do { } while (0)
#endif

#endif