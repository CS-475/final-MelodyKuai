#include "blend_modes.h"
#include "./include/GPixel.h"

// Blend function implementations
#include "blend_modes.h"
#include "./include/GPixel.h"
#include <algorithm>  // for std::max and std::min

// Blend function implementations

// kClear: Fully transparent pixel
GPixel clear_mode(GPixel src, GPixel dst) {
    return GPixel_PackARGB(0, 0, 0, 0);
}

// kSrc: Use the source pixel directly
GPixel src_mode(GPixel src, GPixel dst) {
    return src;
}

// kDst: Use the destination pixel directly
GPixel dst_mode(GPixel src, GPixel dst) {
    return dst;
}

// kSrcOver: S + (1 - Sa) * D
GPixel src_over_mode(GPixel src, GPixel dst) {
    int srcA = GPixel_GetA(src);
    if (srcA == 255) {
        // Full opacity, return src directly
        return src;
    }
    if (srcA == 0) {
        return dst;
    }
    int invSrcA = 255 - srcA;

    
    int a = srcA + ((invSrcA * GPixel_GetA(dst)) >> 8);
    int r = GPixel_GetR(src) + ((invSrcA * GPixel_GetR(dst)) >> 8);
    int g = GPixel_GetG(src) + ((invSrcA * GPixel_GetG(dst)) >> 8);
    int b = GPixel_GetB(src) + ((invSrcA * GPixel_GetB(dst)) >> 8);
    
    return GPixel_PackARGB(a, r, g, b);
}

// kDstOver: D + (1 - Da) * S
GPixel dst_over_mode(GPixel src, GPixel dst) {
    return src_over_mode(dst, src);  // Swap src and dst to reuse the src_over_mode
}

// kSrcIn: Da * S
GPixel src_in_mode(GPixel src, GPixel dst) {
    int dstA = GPixel_GetA(dst);

    
    int a = (GPixel_GetA(src) * dstA) >> 8;
    int r = (GPixel_GetR(src) * dstA) >> 8;
    int g = (GPixel_GetG(src) * dstA) >> 8;
    int b = (GPixel_GetB(src) * dstA) >> 8;

    return GPixel_PackARGB(a, r, g, b);
}

// kDstIn: Sa * D
GPixel dst_in_mode(GPixel src, GPixel dst) {
    int srcA = GPixel_GetA(src);

    
    int a = (GPixel_GetA(dst) * srcA) >> 8;
    int r = (GPixel_GetR(dst) * srcA) >> 8;
    int g = (GPixel_GetG(dst) * srcA) >> 8;
    int b = (GPixel_GetB(dst) * srcA) >> 8;

    return GPixel_PackARGB(a, r, g, b);
}

// kSrcOut: (1 - Da) * S
GPixel src_out_mode(GPixel src, GPixel dst) {
    int dstA = GPixel_GetA(dst);
    int invDstA = 255 - dstA;

    
    int a = (GPixel_GetA(src) * invDstA) >> 8;
    int r = (GPixel_GetR(src) * invDstA) >> 8;
    int g = (GPixel_GetG(src) * invDstA) >> 8;
    int b = (GPixel_GetB(src) * invDstA) >> 8;

    return GPixel_PackARGB(a, r, g, b);
}

// kDstOut: (1 - Sa) * D
GPixel dst_out_mode(GPixel src, GPixel dst) {
    int srcA = GPixel_GetA(src);
    int invSrcA = 255 - srcA;

    
    int a = (GPixel_GetA(dst) * invSrcA) >> 8;
    int r = (GPixel_GetR(dst) * invSrcA) >> 8;
    int g = (GPixel_GetG(dst) * invSrcA) >> 8;
    int b = (GPixel_GetB(dst) * invSrcA) >> 8;

    return GPixel_PackARGB(a, r, g, b);
}

// kSrcATop: Da * S + (1 - Sa) * D
GPixel src_atop_mode(GPixel src, GPixel dst) {
    int srcA = GPixel_GetA(src);
    int dstA = GPixel_GetA(dst);
    int invSrcA = 255 - srcA;

    
    int a = (dstA * srcA + invSrcA * GPixel_GetA(dst)) >> 8;
    int r = (dstA * GPixel_GetR(src) + invSrcA * GPixel_GetR(dst)) >> 8;
    int g = (dstA * GPixel_GetG(src) + invSrcA * GPixel_GetG(dst)) >> 8;
    int b = (dstA * GPixel_GetB(src) + invSrcA * GPixel_GetB(dst)) >> 8;

    return GPixel_PackARGB(a, r, g, b);
}

// kDstATop: Sa * D + (1 - Da) * S
GPixel dst_atop_mode(GPixel src, GPixel dst) {
    int srcA = GPixel_GetA(src);
    int dstA = GPixel_GetA(dst);
    int invDstA = 255 - dstA;

    
    int a = (srcA * dstA + invDstA * GPixel_GetA(src)) >> 8;
    int r = (srcA * GPixel_GetR(dst) + invDstA * GPixel_GetR(src)) >> 8;
    int g = (srcA * GPixel_GetG(dst) + invDstA * GPixel_GetG(src)) >> 8;
    int b = (srcA * GPixel_GetB(dst) + invDstA * GPixel_GetB(src)) >> 8;

    return GPixel_PackARGB(a, r, g, b);
}

// kXor: (1 - Sa) * D + (1 - Da) * S
GPixel xor_mode(GPixel src, GPixel dst) {
    int srcA = GPixel_GetA(src);
    int dstA = GPixel_GetA(dst);
    int invSrcA = 255 - srcA;
    int invDstA = 255 - dstA;

        
    int a = (invSrcA * dstA + invDstA * srcA) >> 8;
    int r = (invSrcA * GPixel_GetR(dst) + invDstA * GPixel_GetR(src)) >> 8;
    int g = (invSrcA * GPixel_GetG(dst) + invDstA * GPixel_GetG(src)) >> 8;
    int b = (invSrcA * GPixel_GetB(dst) + invDstA * GPixel_GetB(src)) >> 8;

    return GPixel_PackARGB(a, r, g, b);
}

// Lookup table for blend functions
const BlendProc gProcs[] = {
    clear_mode,    // kClear
    src_mode,      // kSrc
    dst_mode,      // kDst
    src_over_mode, // kSrcOver
    dst_over_mode, // kDstOver
    src_in_mode,   // kSrcIn
    dst_in_mode,   // kDstIn
    src_out_mode,  // kSrcOut
    dst_out_mode,  // kDstOut
    src_atop_mode, // kSrcATop
    dst_atop_mode, // kDstATop
    xor_mode       // kXor
};
