#ifndef GFinalCustom_DEFINED
#define GFinalCustom_DEFINED

#include "./include/GShader.h"
#include "./include/GCanvas.h"
#include "./include/GPath.h"
#include "./include/GColor.h"
#include "./include/GFinal.h"
#include "./include/GPoint.h"
#include "./include/GMatrix.h"
#include "./my_utils.h"
#include <memory>
#include <vector>

class GFinalCustom : public GFinal {
public:
    virtual ~GFinalCustom() {}

    std::shared_ptr<GShader> createVoronoiShader(const GPoint points[], const GColor colors[], int count) override;

    std::shared_ptr<GShader> createSweepGradient(GPoint center, float startRadians, const GColor colors[], int count) override;

    std::shared_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) override;

    std::shared_ptr<GShader> createColorMatrixShader(const GColorMatrix& matrix, GShader* realShader) override;

    std::shared_ptr<GPath> strokePolygon(const GPoint points[], int count, float width, bool isClosed) override;

    void drawQuadraticCoons(GCanvas* canvas, const GPoint pts[8], const GPoint tex[4], int level, const GPaint& paint) override;
};

std::unique_ptr<GFinal> GCreateFinal();


class GSweepGradientShader : public GShader {
public:
    GSweepGradientShader(GPoint center, float startRadians, const GColor* colors, int count)
        : fCenter(center), fStartRadians(startRadians), fColors(colors), fCount(count) {}

    bool isOpaque() override {
        return false;
    }

    bool setContext(const GMatrix& ctm) override {
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; ++i) {
            float dx = x - fCenter.x;
            float dy = y - fCenter.y;
            float angle = std::atan2(dy, dx);
            if (angle < 0) angle += static_cast<float>(2 * M_PI);

            float normalizedAngle = static_cast<float>((angle - fStartRadians) / (2 * M_PI));
            if (normalizedAngle < 0) normalizedAngle += 1.0f;
            int colorIndex = static_cast<int>(normalizedAngle * fCount);
            colorIndex = (colorIndex == fCount) ? 0 : colorIndex;

            row[i] = GColorToPixel(fColors[colorIndex]);
        }
    }

private:
    GPoint fCenter;
    float fStartRadians;
    const GColor* fColors;
    int fCount;
};

class GLinearPosGradientShader : public GShader {
public:
    GLinearPosGradientShader(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count)
        : fP0(p0), fP1(p1), fCount(count) {
        
        fColors.reserve(count);
        fPos.reserve(count);
        for (int i = 0; i < count; ++i) {
            fColors.push_back(colors[i]);
            fPos.push_back(pos[i]);
        }
    }

    bool isOpaque() override {
        for (const auto& color : fColors) {
            if (color.a < 1.0f) return false;
        }
        return true;
    }

    bool setContext(const GMatrix& ctm) override {
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; ++i) {
            float t = (x + i - fP0.x) / (fP1.x - fP0.x);
            t = std::min(1.0f, std::max(0.0f, t));
            
            int lower = 0;
            int upper = fCount - 1;
            while (upper - lower > 1) {
                int mid = (upper + lower) / 2;
                if (fPos[mid] < t) {
                    lower = mid;
                } else {
                    upper = mid;
                }
            }
            
            float blend = (t - fPos[lower]) / (fPos[upper] - fPos[lower]);
            GColor blendedColor = interpolate(fColors[lower], fColors[upper], blend);

            row[i] = GColorToPixel(blendedColor);
        }
    }

private:
    GColor interpolate(const GColor& c0, const GColor& c1, float t) {
        return {
            c0.r + t * (c1.r - c0.r),
            c0.g + t * (c1.g - c0.g),
            c0.b + t * (c1.b - c0.b),
            c0.a + t * (c1.a - c0.a)
        };
    }

    GPoint fP0, fP1;
    int fCount;
    std::vector<GColor> fColors;
    std::vector<float> fPos;
};

class GColorMatrixShader : public GShader {
public:
    GColorMatrixShader(const GColorMatrix& matrix, GShader* realShader)
        : fMatrix(matrix), fRealShader(realShader) {}

    bool isOpaque() override {
        return false;
    }

    bool setContext(const GMatrix& ctm) override {
        return fRealShader->setContext(ctm);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        std::vector<GPixel> realRow(count);
        fRealShader->shadeRow(x, y, count, realRow.data());

        for (int i = 0; i < count; ++i) {
            GColor color = GPixelToColor(realRow[i]);

            float r = fMatrix[0] * color.r + fMatrix[4] * color.g + fMatrix[8] * color.b + fMatrix[12] * color.a + fMatrix[16];
            float g = fMatrix[1] * color.r + fMatrix[5] * color.g + fMatrix[9] * color.b + fMatrix[13] * color.a + fMatrix[17];
            float b = fMatrix[2] * color.r + fMatrix[6] * color.g + fMatrix[10] * color.b + fMatrix[14] * color.a + fMatrix[18];
            float a = fMatrix[3] * color.r + fMatrix[7] * color.g + fMatrix[11] * color.b + fMatrix[15] * color.a + fMatrix[19];

            r = std::clamp(r, 0.0f, 1.0f);
            g = std::clamp(g, 0.0f, 1.0f);
            b = std::clamp(b, 0.0f, 1.0f);
            a = std::clamp(a, 0.0f, 1.0f);

            row[i] = GColorToPixel({r, g, b, a});
        }
    }

private:
    GColorMatrix fMatrix;
    GShader* fRealShader;

    GColor GPixelToColor(GPixel pixel) {
        float r = ((pixel >> 16) & 0xFF) / 255.0f;
        float g = ((pixel >> 8) & 0xFF) / 255.0f;
        float b = (pixel & 0xFF) / 255.0f;
        float a = ((pixel >> 24) & 0xFF) / 255.0f;
        return {r, g, b, a};
    }
};


#endif  // GFinalCustom_DEFINED

