#ifndef MY_UTILS_H
#define MY_UTILS_H

#include "./include/GColor.h"
#include "./include/GPixel.h"
#include "./include/GMatrix.h"
#include "./include/GPoint.h"

// Utility function to convert GColor to GPixel
GPixel GColorToPixel(const GColor& color);

// Utility function to blend two colors using SRC_OVER mode
GPixel BlendPixels(const GPixel& src, const GPixel& dst);

GMatrix compute_basis(const GPoint& p0, const GPoint& p1, const GPoint& p2);

#endif // MY_UTILS_H