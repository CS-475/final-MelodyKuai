#ifndef PROXY_SHADER_H
#define PROXY_SHADER_H

#include "./include/GShader.h"
#include "./include/GMatrix.h"
#include <memory>

class ProxyShader : public GShader {
    std::shared_ptr<GShader> fRealShader;
    GMatrix fExtraTransform;

public:
    ProxyShader(std::shared_ptr<GShader> shader, const GMatrix& extraTransform)
        : fRealShader(shader), fExtraTransform(extraTransform) {}

    bool isOpaque() override {
        return fRealShader->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        return fRealShader->setContext(GMatrix::Concat(ctm, fExtraTransform));
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        fRealShader->shadeRow(x, y, count, row);
    }
};

#endif
