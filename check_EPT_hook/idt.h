#pragma once
#include <ntddk.h>

struct trap_frame {
    // TODO: SSE registers...

    // general-purpose registers
    union {
        UINT64 rax;
        UINT32 eax;
        UINT16 ax;
        UINT8  al;
    };
    union {
        UINT64 rcx;
        UINT32 ecx;
        UINT16 cx;
        UINT8  cl;
    };
    union {
        UINT64 rdx;
        UINT32 edx;
        UINT16 dx;
        UINT8  dl;
    };
    union {
        UINT64 rbx;
        UINT32 ebx;
        UINT16 bx;
        UINT8  bl;
    };
    union {
        UINT64 rbp;
        UINT32 ebp;
        UINT16 bp;
        UINT8  bpl;
    };
    union {
        UINT64 rsi;
        UINT32 esi;
        UINT16 si;
        UINT8  sil;
    };
    union {
        UINT64 rdi;
        UINT32 edi;
        UINT16 di;
        UINT8  dil;
    };
    union {
        UINT64 r8;
        UINT32 r8d;
        UINT16 r8w;
        UINT8  r8b;
    };
    union {
        UINT64 r9;
        UINT32 r9d;
        UINT16 r9w;
        UINT8  r9b;
    };
    union {
        UINT64 r10;
        UINT32 r10d;
        UINT16 r10w;
        UINT8  r10b;
    };
    union {
        UINT64 r11;
        UINT32 r11d;
        UINT16 r11w;
        UINT8  r11b;
    };
    union {
        UINT64 r12;
        UINT32 r12d;
        UINT16 r12w;
        UINT8  r12b;
    };
    union {
        UINT64 r13;
        UINT32 r13d;
        UINT16 r13w;
        UINT8  r13b;
    };
    union {
        UINT64 r14;
        UINT32 r14d;
        UINT16 r14w;
        UINT8  r14b;
    };
    union {
        UINT64 r15;
        UINT32 r15d;
        UINT16 r15w;
        UINT8  r15b;
    };

    // interrupt vector
    UINT64 vector;

    // _MACHINE_FRAME
    UINT64 error;
    UINT64 rip;
    UINT64 cs;
    UINT64 rflags;
    UINT64 rsp;         //only r3?
    UINT64 ss;          //only r3?
};

// remember to update this value in interrupt-handlers.asm
static_assert(sizeof(trap_frame) == (0x78 + 0x38), "");



extern "C" void interrupt_handler_01_DB();
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef struct
{
    uint16_t offset_low;
    uint16_t segment_selector;
    union
    {
        struct
        {
            uint32_t interrupt_stack_table : 3;
            uint32_t must_be_zero_0 : 5;
            uint32_t type : 4;
            uint32_t must_be_zero_1 : 1;
            uint32_t descriptor_privilege_level : 2;
            uint32_t present : 1;
            uint32_t offset_middle : 16;
        };

        uint32_t flags;
    };
    uint32_t offset_high;
    uint32_t reserved;
} IDTE;

void ALvmSetIdteFunAdd(IDTE* idte_, UINT64 FunctionAdd);
