#ifndef MARKOV_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef s32 b32;
typedef float f32;
typedef double f64;
typedef intptr_t smm;
typedef uintptr_t umm;

#define Assert(Condition) if(!(Condition)) { *(int *)0 = 0; }
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define MAX(A, B) ((A) > (B) ? A : B)
#define MIN(A, B) ((A) < (B) ? A : B)

#define Kilobytes(Size) (Size * 1024LL)
#define Megabytes(Size) (Kilobytes(Size) * 1024LL)

inline void
ZeroSize(void *Memory, u32 Size)
{
    u8 *At = (u8 *)Memory;
    while(Size--)
    {
        *At++ = 0;
    }
}

struct memory_block
{
    u8 *Base;
    umm Used;
    umm Size;
    struct memory_block *PreviousBlock;
};

struct memory_arena
{
    struct memory_block *CurrentBlock;
    umm MinimumBlockSize;
};

static struct memory_arena
InitializeArena(umm Size)
{
    struct memory_arena Result = {};
    Result.MinimumBlockSize = Size;
    return(Result);
}

#define PushStruct(Arena, Type) (Type *)PushSize_(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type *)PushSize_(Arena, (Count)*sizeof(Type))
static u8 *
PushSize_(memory_arena *Arena, umm Size)
{
    if(!Arena->CurrentBlock ||
       ((Arena->CurrentBlock->Used + Size) > Arena->CurrentBlock->Size))
    {
        umm BlockSize = MAX(Arena->MinimumBlockSize, Size);
        struct memory_block *NewBlock = (struct memory_block *)malloc(BlockSize + sizeof(*NewBlock));
        Assert(NewBlock != 0);
        ZeroSize(NewBlock, BlockSize + sizeof(*NewBlock));
        NewBlock->Base = (u8 *)(NewBlock + 1);
        NewBlock->Size = BlockSize;
        NewBlock->PreviousBlock = Arena->CurrentBlock;
        Arena->CurrentBlock = NewBlock;
    }

    Assert(Arena->CurrentBlock->Used + Size < Arena->CurrentBlock->Size);

    u8 *Result = Arena->CurrentBlock->Base + Arena->CurrentBlock->Used;
    Arena->CurrentBlock->Used += Size;

    return(Result);
}

static inline b32
IsWhitespace(char C)
{
    b32 Result = ( (C == ' ') ||
                   (C == '\r') ||
                   (C == '\n') ||
                   (C == '\t') );
    return(Result);
}

static inline b32
IsWhitespaceOrNull(char C)
{
    b32 Result = (IsWhitespace(C) || (C == 0));
    return(Result);
}

static inline void
ConsumeWhitespace(char **AtPtr)
{
    char *At = *AtPtr;
    while(IsWhitespace(*At))
    {
        ++At;
    }

    *AtPtr = At;
}

static inline u32
StringLength(char *Source)
{
    u32 Result = 0;
    for( ; *Source != 0; ++Source, ++Result) ;
    return(Result);
}

#define MARKOV_H
#endif
