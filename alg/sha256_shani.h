#ifndef _SHA256_SHANI_H_
#define _SHA256_SHANI_H_

#include <stdint.h>

/*
 * SHA256_Transform_shani(state, block):
 * SHA256 block compression function.  The 256-bit state is transformed via
 * the 512-bit input data block to produce a new state.  This implementation
 * uses x86 SHANI instructions, and should only be used if CPUSUPPORT_X86_SHANI
 * is defined and cpusupport_x86_shani() returns nonzero.
 */
void SHA256_Transform_shani(uint32_t state[static restrict 8],
    const uint8_t block[static restrict 64]);

#endif
