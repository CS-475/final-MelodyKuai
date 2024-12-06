#include "./GFinalCustom.h"

#include <cmath>

std::shared_ptr<GShader> GFinalCustom::createVoronoiShader(const GPoint points[], const GColor colors[], int count) {
    return nullptr;  
}

std::shared_ptr<GShader> GFinalCustom::createSweepGradient(GPoint center, float startRadians, const GColor colors[], int count) {
    if (count <= 0) {
        return nullptr;
    }

    return std::make_shared<GSweepGradientShader>(center, startRadians, colors, count);
}

std::shared_ptr<GShader> GFinalCustom::createLinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) {
    return std::make_shared<GLinearPosGradientShader>(p0, p1, colors, pos, count);
}

std::shared_ptr<GShader> GFinalCustom::createColorMatrixShader(const GColorMatrix& matrix, GShader* realShader) {
    return std::make_shared<GColorMatrixShader>(matrix, realShader);
}

std::shared_ptr<GPath> GFinalCustom::strokePolygon(const GPoint points[], int count, float width, bool isClosed) {
    return nullptr;  
}

void GFinalCustom::drawQuadraticCoons(GCanvas* canvas, const GPoint pts[8], const GPoint tex[4], int level, const GPaint& paint) {

}

std::unique_ptr<GFinal> GCreateFinal() {
    return std::make_unique<GFinalCustom>();
}