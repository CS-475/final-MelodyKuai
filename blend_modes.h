#ifndef BLEND_MODES_H
#define BLEND_MODES_H

#include "./include/GPixel.h"
#include "./include/GBlendMode.h"

// Define a function pointer type for blending functions
typedef GPixel (*BlendProc)(GPixel src, GPixel dst);

// Declare each blend function
GPixel clear_mode(GPixel src, GPixel dst);
GPixel src_mode(GPixel src, GPixel dst);
GPixel dst_mode(GPixel src, GPixel dst);
GPixel src_over_mode(GPixel src, GPixel dst);
GPixel dst_over_mode(GPixel src, GPixel dst);
GPixel src_in_mode(GPixel src, GPixel dst);
GPixel dst_in_mode(GPixel src, GPixel dst);
GPixel src_out_mode(GPixel src, GPixel dst);
GPixel dst_out_mode(GPixel src, GPixel dst);
GPixel src_atop_mode(GPixel src, GPixel dst);
GPixel dst_atop_mode(GPixel src, GPixel dst);
GPixel xor_mode(GPixel src, GPixel dst);

// Lookup table of blend functions
extern const BlendProc gProcs[];

#endif  // BLEND_MODES_H