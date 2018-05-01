#include "cpusupport.h"
#ifdef CPUSUPPORT_X86_CRC32

#include <smmintrin.h>

#include "crc32c_sse42.h"

#if __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wcast-align\"")
#endif
/**
 * CRC32C_Update_SSE42(state, buf, len):
 * Feed ${len} bytes from the buffer ${buf} into the CRC32C whose state is
 * ${state}.  This implentation uses x86 SSE4.2 instructions, and should only
 * be used if CPUSUPPORT_X86_CRC32 is defined and cpusupport_x86_crc32 returns
 * nonzero.
 */
void
CRC32C_Update_SSE42(uint32_t * state, const uint8_t * buf, size_t len)
{
	const size_t pre = ((uintptr_t)buf & 7) < len ? (uintptr_t)buf & 7 : len;
	const size_t remaining = len - pre;
	const size_t block_len = remaining - remaining % 8;
	size_t i = 0;

	/* Process bytes before the alignment. */
	for (; i < pre; i++)
		*state = _mm_crc32_u8(*state, buf[i]);

	/* Process blocks of 8. */
	for (; i < block_len; i += 8) {
		*state = (uint32_t)_mm_crc32_u64(*state,
		    *(const unsigned long *)&buf[i]);
	}

	/* Process any remaining bytes. */
	for (; i < len; i++)
		*state = _mm_crc32_u8(*state, buf[i]);
}
#if __clang__
_Pragma("clang diagnostic pop")
#endif

#endif /* CPUSUPPORT_X86_CRC32 */
