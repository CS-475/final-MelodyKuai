#ifndef MY_GPATH_H
#define MY_GPATH_H

#include "./include/GPathBuilder.h"
#include <algorithm>

void computeQuadBounds(const GPoint pts[3], float& minX, float& maxX, float& minY, float& maxY);

void computeCubicBounds(const GPoint pts[4], float& minX, float& maxX, float& minY, float& maxY);

float cubicAt(float A, float B, float C, float D, float t);

int solveQuadratic(float a, float b, float c, float roots[2]);

#endif // MY_GPATH_H