/**
 * @author Wayne Moorefield
 * @brief This file contains configuration parameters
 */

#ifndef _EWATC_SOCCFG_H
#define _EWATC_SOCCFG_H

#define MEMORY_SIZE   0x0080
#define MEMORY_RESET_VALUE 0xFF

#define MAX_CPU_REGISTERS 3
#define CPU_REGISTER_RESET_VALUE 0x1A1A2B2B
#define CPU_PC_RESET_VECTOR 0x00000000
#define CPU_INSTRUCTION_SIZE 4


// This section do not worry about

// Intel is Little Endian
// ARM is Big Endian
// More information: http://en.wikipedia.org/wiki/Endianness
#define SOC_BIG_ENDIAN
#define HOST_LITTLE_ENDIAN

#if 0
/* This is some defines that determine at compile time the host
 * endianess. It does cause a warning and is not cross-compiler
 * friendly.
 */
#define LITTLE_ENDIAN 0x41424344UL
#define BIG_ENDIAN    0x44434241UL
#define ENDIAN_ORDER  ('ABCD')
#undef HOST_LITTLE_ENDIAN
#undef HOST_BIG_ENDIAN

#if ENDIAN_ORDER==LITTLE_ENDIAN
    #define HOST_LITTLE_ENDIAN
#elif ENDIAN_ORDER==BIG_ENDIAN
    #define HOST_BIG_ENDIAN
#else
    #error "Unable to determine host endianness"
#endif
#endif


#endif
