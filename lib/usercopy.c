// SPDX-License-Identifier: GPL-2.0
#include <linux/uaccess.h>
#include <linux/nospec.h>

/* out-of-line parts */

#ifndef INLINE_COPY_FROM_USER
unsigned long _copy_from_user(void *to, const void __user *from, unsigned long n)
{
	unsigned long res = n;
	might_fault();
	if (likely(access_ok(VERIFY_READ, from, n))) {
		/*
		 * Ensure that bad access_ok() speculation will not
		 * lead to nasty side effects *after* the copy is
		 * finished:
		 */
		barrier_nospec();
		kasan_check_write(to, n);
		res = raw_copy_from_user(to, from, n);
	}
	if (unlikely(res))
		memset(to + (n - res), 0, res);
	return res;
}
EXPORT_SYMBOL(_copy_from_user);
#endif

#ifndef INLINE_COPY_TO_USER
unsigned long _copy_to_user(void __user *to, const void *from, unsigned long n)
{
	might_fault();
	if (likely(access_ok(VERIFY_WRITE, to, n))) {
		kasan_check_read(from, n);
		n = raw_copy_to_user(to, from, n);
	}
	return n;
}
EXPORT_SYMBOL(_copy_to_user);
#endif

/*
 * check_zeroed_user: check if a userspace buffer only contains zero bytes
 * @from: Source address, in userspace.
 * @size: Size of buffer.
 *
 * 4.14 兼容版: LOS 原实现依赖 5.x 的 user_access_begin/unsafe_get_user,
 * 这里用 __get_user 等价实现(语义相同, 无 unsafe 优化)。
 *
 * Returns:
 *  * 0: There were non-zero bytes present in the buffer.
 *  * 1: The buffer was full of zero bytes.
 *  * -EFAULT: access to userspace failed.
 */
int check_zeroed_user(const void __user *from, size_t size)
{
	unsigned long val;
	uintptr_t align = (uintptr_t)from % sizeof(unsigned long);

	if (unlikely(size == 0))
		return 1;

	from -= align;
	size += align;

	if (!access_ok(VERIFY_READ, from, size))
		return -EFAULT;

	if (__get_user(val, (unsigned long __user *)from))
		return -EFAULT;
	if (align)
		val &= ~aligned_byte_mask(align);

	while (size > sizeof(unsigned long)) {
		if (unlikely(val))
			goto done;

		from += sizeof(unsigned long);
		size -= sizeof(unsigned long);

		if (__get_user(val, (unsigned long __user *)from))
			return -EFAULT;
	}

	if (size < sizeof(unsigned long))
		val &= aligned_byte_mask(size);

done:
	return (val == 0);
}
