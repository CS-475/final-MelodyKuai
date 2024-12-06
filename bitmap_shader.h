#ifndef BITMAP_SHADER_H
#define BITMAP_SHADER_H

#include "./include/GShader.h"
#include "./include/GBitmap.h"
#include "./include/GMatrix.h"

class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode);
    bool isOpaque() override;
    bool setContext(const GMatrix& ctm) override;
    void shadeRow(int x, int y, int count, GPixel row[]) override;
private:
    GBitmap fBitmap;
    GMatrix fInverse;
    GMatrix fLocalMatrix;
    bool fOpaque;
    GTileMode fTileMode;
};

std::shared_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode);

#endif