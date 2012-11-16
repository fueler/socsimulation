/**
 * @author Wayne Moorefield
 * @brief This file contains common prototypes
 */

#include "types.h"
#include "soccfg.h"

#define REG_PC (MAX_CPU_REGISTERS - 1)
#define REG_SP (MAX_CPU_REGISTERS - 2)

union TestInstruction {
    U32 value32;
    U16 value16[2];
    U8  value8[4];

    struct {
#ifdef HOST_LITTLE_ENDIAN
        U16 data;
        U8 regIndex;
        U8 opcode;
#elif defined(HOST_BIG_ENDIAN)
        U8 opcode;
        U8 regIndex;
        U16 data;
#else
    #error "Invalid Host Configuration"
#endif
    } format1;

    struct {
#ifdef HOST_LITTLE_ENDIAN
        U8 regIndex3;
        U8 regIndex2;
        U8 regIndex1;
        U8 opcode;
#elif defined(HOST_BIG_ENDIAN)
        U8 opcode;
        U8 regIndex1;
        U8 regIndex2;
        U8 regIndex3;
#else
    #error "Invalid Host Configuration"
#endif
    } format2;

    struct {
#ifdef HOST_LITTLE_ENDIAN
        U8 data;
        U8 regIndex2;
        U8 regIndex1;
        U8 opcode;
#elif defined(HOST_BIG_ENDIAN)
        U8 opcode;
        U8 regIndex1;
        U8 regIndex2;
        U8 data;
#else
    #error "Invalid Host Configuration"
#endif
    } format3;
};

struct Memory
{
    U8 data[MEMORY_SIZE];
};

struct CPUContext
{
    U32 reg[MAX_CPU_REGISTERS];
};


bool resetSoC(CPUContext &ctx, Memory &mem);
bool loadProgram(Memory &mem);
bool executeCPUInstruction(CPUContext &ctx, Memory &mem);

bool runProgram(CPUContext &ctx, Memory &mem);

void debugDumpCPU(const CPUContext &ctx);
void debugDumpCPU(const Memory &mem, U32 first=0, U32 last=MEMORY_SIZE-1);
void debugDumpSocStatus(const CPUContext &ctx, const Memory &mem);

bool read8Memory(const Memory &mem, U32 address, U8 &value);
bool read16Memory(const Memory &mem, U32 address, U16 &value);
bool read32Memory(const Memory &mem, U32 address, U32 &value);

bool write8Memory(Memory &mem, U32 address, const U8 &value);
bool write16Memory(Memory &mem, U32 address, const U16 &value);
bool write32Memory(Memory &mem, U32 address, const U32 &value);


TestInstruction buildCPUInstructionFmt1(U8 opcode,
                                        U8 regIndex,
                                        U16 data);
TestInstruction buildCPUInstructionFmt2(U8 opcode,
                                        U8 regIndex1,
                                        U8 regIndex2,
                                        U8 regIndex3);
TestInstruction buildCPUInstructionFmt3(U8 opcode,
                                        U8 regIndex1,
                                        U8 regIndex2,
                                        U8 data);

#define swap16(_value) ((((_value)&0x00FF) << 8) | \
                        (((_value)&0xFF00) >> 8))

#define swap32(_value) ((((_value)&0x000000FF) << 24) | \
                        (((_value)&0x0000FF00) << 8)  | \
                        (((_value)&0x00FF0000) >> 8)  | \
                        (((_value)&0xFF000000) >> 24))

#ifdef HOST_LITTLE_ENDIAN
    #ifdef SOC_LITTLE_ENDIAN
        #define byteswap16(_value) (_value)
        #define byteswap32(_value) (_value)
    #elif defined(SOC_BIG_ENDIAN)
        #define byteswap16(_value) swap16(_value)
        #define byteswap32(_value) swap32(_value)
    #else
        #error "Invalid SoC Configuration"
    #endif
#elif defined(HOST_BIG_ENDIAN)
    #ifdef SOC_LITTLE_ENDIAN
        #define byteswap16(_value) swap16(_value)
        #define byteswap32(_value) swap32(_value)
    #elif defined(SOC_BIG_ENDIAN)
        #define byteswap16(_value) (_value)
        #define byteswap32(_value) (_value)
    #else
        #error "Invalid SoC Configuration"
    #endif
#else
    #error "Invalid Host Configuration"
#endif
