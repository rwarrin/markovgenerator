#ifndef RANDOM_H

static u32 GlobalPRNG;

inline u32
RandomNumber()
{
    u32 Temp = GlobalPRNG;
    Temp ^= Temp << 13;
    Temp ^= Temp >> 17;
    Temp ^= Temp << 5;
    return(GlobalPRNG = Temp);
}

#define RANDOM_H
#endif
