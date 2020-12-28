#ifndef _ALIGN_PTR_H_
#define _ALIGN_PTR_H_

/**
 * ALIGN_PTR_DECL(type, name, num, alignsize):
 * Declare a pointer called ${name}, which points to an array large enough to
 * contain ${num} values of ${type} and is aligned to ${alignsize} bytes.  The
 * pointer must not be used until ALIGN_PTR_INIT(${name}, ${alignsize}) has
 * been called.  This macro may also create an additional variable called
 * "${name}_buf".
 */
#define ALIGN_PTR_DECL(type, name, num, alignsize)			\
	uint8_t name##_buf[num * sizeof(type) + (alignsize - 1)];	\
	type * name

/**
 * ALIGN_PTR_INIT(name, alignsize):
 * Initialize the variable called ${name} to point to an array which is
 * aligned to ${alignsize} bytes.  They must have previously been declared
 * with ALIGN_PTR_DECL(${name}, ${alignsize}).
 */
#define ALIGN_PTR_INIT(name, alignsize)					\
	name = align_ptr(name##_buf, alignsize)

/**
 * align_ptr(arr, alignment):
 * Return a pointer to the first memory location within ${arr} which is
 * aligned to ${alignsize} bytes.  (It is expected that this function will
 * only be called via the ALIGN_PTR_INIT macro).
 */
static inline void *
align_ptr(uint8_t * arr, size_t alignment)
{
	size_t offset;

	offset = (uintptr_t)(&arr[0]) % alignment;
	offset = (alignment - offset) % alignment;
	return ((void *)&arr[offset]);
}

#endif /* !_ALIGN_PTR_H_ */
