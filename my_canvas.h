#ifndef MY_CANVAS_H
#define MY_CANVAS_H

#include "./include/GCanvas.h"
#include "./include/GBitmap.h"
#include "./include/GPaint.h"
#include "./include/GRect.h"
#include "./include/GShader.h"
#include "./include/GPath.h"
#include "./include/GPathBuilder.h"
#include "my_utils.h"
#include "blend_modes.h"
#include "linear_gradient_shader.h"
#include "proxy_shader.h"
#include "composite_shader.h"
#include "bitmap_shader.h"
#include <stack>

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device), fCTM(GMatrix()) {}

    void save() override;
    void restore() override;
    void concat(const GMatrix& matrix) override;
    void clear(const GColor& color) override;
    void drawRect(const GRect& rect, const GPaint& paint) override;
    void drawConvexPolygon(const GPoint pts[], int count, const GPaint& paint) override;

   void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint&);
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                          int level, const GPaint&);

    void blit(int x, int y, int width, const GPaint& paint);
    void drawPath(const GPath& path, const GPaint& paint);

private:
    GBitmap fDevice;
    GMatrix fCTM;
    std::stack<GMatrix> fMatrixStack;

    GBitmap fBitmap;           // Bitmap for texture shaders
    GMatrix fLocalMatrix;      // Local matrix for transformations

    class Edge {
    public:
        GPoint p0, p1;  
        int winding;    

        // Constructor initializes the edge and determines its winding
        Edge(const GPoint& pt0, const GPoint& pt1) {
            if (pt0.y < pt1.y) {
                p0 = pt0;
                p1 = pt1;
                winding = 1;
            } else {
                p0 = pt1;
                p1 = pt0;
                winding = -1;
            }
        }

        int topY() const { return GRoundToInt(p0.y); }
        int bottomY() const { return GRoundToInt(p1.y); }

        // Checks if the edge is valid for a given Y value
        bool isValid(int y) const { return y >= topY() && y < bottomY(); }

        // Computes the X coordinate at the given Y value
        float computeX(int y) const {
            float t = (y - p0.y) / (p1.y - p0.y);
            return p0.x + t * (p1.x - p0.x);
        }

        // Returns the winding value
        int windingValue() const { return winding; }
    };
    void addLineSegment(GPoint p0, GPoint p1, std::vector<Edge>& edges, int& yMin, int& yMax);
    void flattenQuadratic(const GPoint pts[3], std::vector<Edge>& edges, float tolerance, int& yMin, int& yMax);
    void flattenCubic(const GPoint pts[4], std::vector<Edge>& edges, float tolerance, int& yMin, int& yMax);
    void renderEdges(std::vector<Edge>& edges, int yMin, int yMax, const GPaint& paint);
};

#endif