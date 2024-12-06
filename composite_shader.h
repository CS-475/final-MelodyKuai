#ifndef COMPOSITE_SHADER_H
#define COMPOSITE_SHADER_H

#include "./include/GShader.h"

class CompositeShader : public GShader {
public:
    CompositeShader(std::shared_ptr<GShader> s1, std::shared_ptr<GShader> s2)
    : shader1(s1), shader2(s2) {}

    bool isOpaque() override {
        return shader1->isOpaque() && shader2->isOpaque();
    }
    bool setContext(const GMatrix& ctm) override {
        return shader1->setContext(ctm) && shader2->setContext(ctm);
    }
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPixel row1[count], row2[count];
        shader1->shadeRow(x, y, count, row1);
        shader2->shadeRow(x, y, count, row2);

        for (int i = 0; i < count; ++i) {
            int a = GPixel_GetA(row1[i]) * GPixel_GetA(row2[i]) / 255;
            int r = GPixel_GetR(row1[i]) * GPixel_GetR(row2[i]) / 255;
            int g = GPixel_GetG(row1[i]) * GPixel_GetG(row2[i]) / 255;
            int b = GPixel_GetB(row1[i]) * GPixel_GetB(row2[i]) / 255;
            row[i] = GPixel_PackARGB(a, r, g, b);
        }
    }

private:
    std::shared_ptr<GShader> shader1;
    std::shared_ptr<GShader> shader2;
};

#endif
