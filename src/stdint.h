// stdint.h is not yet in the latest libfxcg release

#ifndef STDINT_H
#define STDINT_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned char uint8_t;
    static_assert(sizeof(uint8_t) == 1, "uint8_t should be 1 byte");
    typedef unsigned short uint16_t;
    static_assert(sizeof(uint16_t) == 2, "uint16_t should be 2 bytes");
    typedef unsigned int uint32_t;
    static_assert(sizeof(uint32_t) == 4, "uint32_t should be 4 bytes");

#ifdef __cplusplus
}
#endif

#endif
