#ifndef CRC32C_SSE42_H_
#define CRC32C_SSE42_H_

#include <stddef.h>
#include <stdint.h>

/**
 * CRC32C_Update_SSE42(state, buf, len):
 * Feed ${len} bytes from the buffer ${buf} into the CRC32C whose state is
 * ${state}.  This implementation uses x86 SSE4.2 instructions, and should only
 * be used if CPUSUPPORT_X86_SSE42 is defined and cpusupport_x86_sse42()
 * returns nonzero.  If CPUSUPPORT_X86_SSE42_64 is also defined, 64-bit
 * instructions may be used.  ${len} must be greater than, or equal to, 8.
 */
uint32_t CRC32C_Update_SSE42(uint32_t, const uint8_t *, size_t);

#endif /* !CRC32C_SSE42_H_ */
