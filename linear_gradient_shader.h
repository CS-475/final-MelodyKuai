#ifndef LINEAR_GRADIENT_SHADER_H
#define LINEAR_GRADIENT_SHADER_H

#include "./include/GShader.h"
#include "./include/GMatrix.h"
#include "./include/GPixel.h"
#include "./include/GColor.h"
#include "./include/GPoint.h"
#include "my_utils.h"
#include <memory>


class LinearGradientShader : public GShader {
public:
    LinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode);
    ~LinearGradientShader();  

    bool isOpaque() override;
    bool setContext(const GMatrix& ctm) override;
    void shadeRow(int x, int y, int count, GPixel row[]) override;

private:
    GPoint fP0, fP1;         
    GColor* fColors;         // Dynamically allocated array to hold gradient colors
    int fCount;              // Number of colors in the gradient
    GMatrix fTotalMatrix;    
    GMatrix fInverseMatrix;  
    GTileMode fTileMode;
};


std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode);

class MyTriColorShader : public GShader {
public:
    MyTriColorShader(const GPoint& p0, const GPoint& p1, const GPoint& p2,
                     const GColor& c0, const GColor& c1, const GColor& c2) 
        : fP0(p0), fP1(p1), fP2(p2), fC0(c0), fC1(c1), fC2(c2) {}

    bool isOpaque() override {
        return fC0.a == 1 && fC1.a == 1 && fC2.a == 1;
    }

    bool setContext(const GMatrix& ctm) override {
        // Compute the transformation matrix from the triangle points to unit coordinates
        GMatrix triangleMatrix = GMatrix(fP1.x - fP0.x, fP2.x - fP0.x, fP0.x,
                                         fP1.y - fP0.y, fP2.y - fP0.y, fP0.y);
        GMatrix totalMatrix = GMatrix::Concat(ctm, triangleMatrix);
        
        // Invert the transformation matrix to map from canvas space to triangle space
        auto inv = totalMatrix.invert();
        if (!inv) {
            return false;
        }
        fInverseMatrix = *inv;
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; ++i) {
            GPoint localPoint = fInverseMatrix * GPoint{x + i + 0.5f, y + 0.5f};

            // Compute barycentric coordinates
            float a = 1 - localPoint.x - localPoint.y;
            float b = localPoint.x;
            float c = localPoint.y;

            // Interpolate colors using barycentric coordinates
            GColor interpolatedColor = {
                a * fC0.r + b * fC1.r + c * fC2.r,
                a * fC0.g + b * fC1.g + c * fC2.g,
                a * fC0.b + b * fC1.b + c * fC2.b,
                a * fC0.a + b * fC1.a + c * fC2.a
            };

            // Convert interpolated color to premultiplied pixel
            row[i] = GColorToPixel(interpolatedColor);
        }
    }

private:
    GPoint fP0, fP1, fP2;
    GColor fC0, fC1, fC2;
    GMatrix fInverseMatrix;
};

#endif 
