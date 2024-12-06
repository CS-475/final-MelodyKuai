#include "linear_gradient_shader.h"
#include <cmath>


LinearGradientShader::LinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode)
    : fP0(p0), fP1(p1), fCount(count), fTileMode(tileMode) {
    fColors = new GColor[fCount];
    for (int i = 0; i < fCount; ++i) {
        fColors[i] = colors[i];
    }
}

LinearGradientShader::~LinearGradientShader() {
    delete[] fColors;  // Release dynamically allocated memory
}

bool LinearGradientShader::isOpaque() {
    for (int i = 0; i < fCount; ++i) {
        if (fColors[i].a != 1.0f) {
            return false;
        }
    }
    return true;
}

bool LinearGradientShader::setContext(const GMatrix& ctm) {
    // Calculate the transformation matrix
    float dx = fP1.x - fP0.x;
    float dy = fP1.y - fP0.y;

    // Set the transformation matrix
    GMatrix gradientMatrix = GMatrix(dx, -dy, fP0.x,
                                     dy,  dx, fP0.y);

    fTotalMatrix = GMatrix::Concat(ctm, gradientMatrix);
    

    auto inv = fTotalMatrix.invert();
    if (!inv) {
        return false; 
    }
    fInverseMatrix = *inv; 
    return true;
}

void LinearGradientShader::shadeRow(int x, int y, int count, GPixel row[]) {
    for (int i = 0; i < count; ++i) {
        GPoint src = { x + i + 0.5f, y + 0.5f };
        fInverseMatrix.mapPoints(&src, &src, 1);
       
        float t = src.x;
        switch (fTileMode) {
            case GTileMode::kClamp:
                t = std::clamp(t, 0.0f, 1.0f);
                break;
            case GTileMode::kRepeat:
                t -= std::floor(t);
                break;
            case GTileMode::kMirror:
                t = std::fabs(std::fmod(t, 2.0f));
                t = (t > 1) ? (2 - t) : t;
                break;
        }

        float scaledT = t * (fCount - 1);
        int index = std::min(static_cast<int>(scaledT), fCount - 2);
        float localT = scaledT - index;

        GColor interpolatedColor = {
            fColors[index].r + localT * (fColors[index + 1].r - fColors[index].r),
            fColors[index].g + localT * (fColors[index + 1].g - fColors[index].g),
            fColors[index].b + localT * (fColors[index + 1].b - fColors[index].b),
            fColors[index].a + localT * (fColors[index + 1].a - fColors[index].a)
        };
        row[i] = GColorToPixel(interpolatedColor);
    }
}


std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode) {
    if (count < 1) return nullptr;  
    if (count == 1) {
        GColor twoColors[2] = { colors[0], colors[0] };  
        return std::make_shared<LinearGradientShader>(p0, p1, twoColors, 2, tileMode);
    }
    return std::make_shared<LinearGradientShader>(p0, p1, colors, count, tileMode);
}
