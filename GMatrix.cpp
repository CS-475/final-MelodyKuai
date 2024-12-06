#include "./include/GMatrix.h"
#include <cmath>

GMatrix::GMatrix() {
    // Identity matrix: [ 1 0 0 ] [ 0 1 0 ] [ 0 0 1 ]
    fMat[0] = 1; fMat[2] = 0; fMat[4] = 0;
    fMat[1] = 0; fMat[3] = 1; fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1, 0, tx, 0, 1, ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx, 0, 0, 0, sy, 0);
}

GMatrix GMatrix::Rotate(float radians) {
    float cosTheta = std::cos(radians);
    float sinTheta = std::sin(radians);
    return GMatrix(cosTheta, -sinTheta, 0, sinTheta, cosTheta, 0);
}

// Concatenate two matrices: result = a * b
GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix(
        a[0] * b[0] + a[2] * b[1], 
        a[0] * b[2] + a[2] * b[3], 
        a[0] * b[4] + a[2] * b[5] + a[4],
        a[1] * b[0] + a[3] * b[1], 
        a[1] * b[2] + a[3] * b[3], 
        a[1] * b[4] + a[3] * b[5] + a[5]
    );
}


nonstd::optional<GMatrix> GMatrix::invert() const {
    double det = fMat[0] * fMat[3] - fMat[1] * fMat[2]; 
    if (det == 0) {
        return {};
    }
    double invDet = 1.0 / det;

    float inv_a = static_cast<float>(fMat[3] * invDet);
    float inv_b = static_cast<float>(-fMat[1] * invDet);
    float inv_c = static_cast<float>(-fMat[2] * invDet);
    float inv_d = static_cast<float>(fMat[0] * invDet);

    float inv_e = - static_cast<float>(inv_a * fMat[4]+inv_c * fMat[5]);
    float inv_f = - static_cast<float>(inv_b * fMat[4]+inv_d * fMat[5]);

    return GMatrix(inv_a, inv_c, inv_e, inv_b, inv_d, inv_f);
}


void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        float x = src[i].x;
        float y = src[i].y;
        dst[i].x = fMat[0] * x + fMat[2] * y + fMat[4];
        dst[i].y = fMat[1] * x + fMat[3] * y + fMat[5];
    }
}