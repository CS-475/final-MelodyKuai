#include "my_utils.h"
#include "./include/GMath.h"

// Converts GColor to GPixel with premultiplied alpha
GPixel GColorToPixel(const GColor& color) {
    float a = GPinToUnit(color.a) * 255.0f + 0.5f;  
    float r = GPinToUnit(color.r * color.a) * 255.0f + 0.5f;  
    float g = GPinToUnit(color.g * color.a) * 255.0f + 0.5f;  
    float b = GPinToUnit(color.b * color.a) * 255.0f + 0.5f;  
    return GPixel_PackARGB(static_cast<int>(a), static_cast<int>(r), static_cast<int>(g), static_cast<int>(b));
}


// Blends two pixels using SRC_OVER mode
GPixel BlendPixels(const GPixel& src, const GPixel& dst) {
    int srcA = GPixel_GetA(src);
    int invSrcA = 255 - srcA;

    int a = srcA + invSrcA * GPixel_GetA(dst) / 255;
    int r = GPixel_GetR(src) + invSrcA * GPixel_GetR(dst) / 255;
    int g = GPixel_GetG(src) + invSrcA * GPixel_GetG(dst) / 255;
    int b = GPixel_GetB(src) + invSrcA * GPixel_GetB(dst) / 255;

    return GPixel_PackARGB(a, r, g, b);
}

GMatrix compute_basis(const GPoint& p0, const GPoint& p1, const GPoint& p2) {
    float Ux = p1.x - p0.x;
    float Uy = p1.y - p0.y;
    float Vx = p2.x - p0.x;
    float Vy = p2.y - p0.y;
    return GMatrix(Ux, Vx, p0.x, Uy, Vy, p0.y);
}