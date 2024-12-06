#include "my_canvas.h"
#include "my_utils.h"
#include "blend_modes.h"
#include "my_gpath.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <climits>

using namespace std;

GPixel Blend(GPixel src, GPixel dst, GBlendMode mode) {
    int srcA = GPixel_GetA(src);  // Get source alpha channel

    // Handle source blending modes
    if (mode == GBlendMode::kSrc) {
        return src;  // In kSrc mode, source completely replaces destination
    } 
    else if (mode == GBlendMode::kSrcOver) {
        if (srcA == 255) {
            return src;  // Fully opaque source, just return it
        } else if (srcA == 0) {
            return dst;  // Fully transparent source, just return destination
        } else {
            // Blend source over destination
            int invSrcA = 255 - srcA;  
            int a = srcA + ((invSrcA * GPixel_GetA(dst)) >> 8);  
            int r = GPixel_GetR(src) + ((invSrcA * GPixel_GetR(dst)) >> 8);  
            int g = GPixel_GetG(src) + ((invSrcA * GPixel_GetG(dst)) >> 8);  
            int b = GPixel_GetB(src) + ((invSrcA * GPixel_GetB(dst)) >> 8);  
            return GPixel_PackARGB(a, r, g, b);  
        }
    }
    // For all other blend modes
    return gProcs[static_cast<int>(mode)](src, dst);
}




// Clears the entire canvas with the given color
void MyCanvas::clear(const GColor& color) {
    GPixel pixel = GColorToPixel(color);
    GPixel* row = fDevice.pixels();
    size_t totalPixels = fDevice.height() * fDevice.width();

    for (size_t i = 0; i < totalPixels; ++i) {
        row[i] = pixel;
    }
}

void MyCanvas::save() {
    fMatrixStack.push(fCTM); // Save current CTM
}

void MyCanvas::restore() {
    if (!fMatrixStack.empty()) {
        fCTM = fMatrixStack.top(); 
        fMatrixStack.pop(); 
    } else {
        std::cerr << "Error: No saved matrix to restore!" << std::endl;
    }
}

void MyCanvas::concat(const GMatrix& matrix) {
    fCTM = GMatrix::Concat(fCTM, matrix); // Pre-concatenate the matrix
}


void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
    GShader* shader = paint.peekShader();
    GBlendMode mode = paint.getBlendMode();

    // First, convert the rectangle into its 4 corner points
    GPoint corners[4] = {
        {rect.left, rect.top},
        {rect.right, rect.top},
        {rect.right, rect.bottom},
        {rect.left, rect.bottom}
    };
    
    // Apply the CTM (current transformation matrix) to all 4 corners
    GPoint transformedCorners[4];
    fCTM.mapPoints(transformedCorners, corners, 4);

    // Now calculate the transformed bounding box (minX, minY, maxX, maxY)
    float minX = transformedCorners[0].x, maxX = transformedCorners[0].x;
    float minY = transformedCorners[0].y, maxY = transformedCorners[0].y;

    for (int i = 1; i < 4; ++i) {
        minX = std::min(minX, transformedCorners[i].x);
        maxX = std::max(maxX, transformedCorners[i].x);
        minY = std::min(minY, transformedCorners[i].y);
        maxY = std::max(maxY, transformedCorners[i].y);
    }

    // Clip the bounding box to the canvas size
    int left = std::max(GRoundToInt(minX), 0);
    int right = std::min(GRoundToInt(maxX), fDevice.width());
    int top = std::max(GRoundToInt(minY), 0);
    int bottom = std::min(GRoundToInt(maxY), fDevice.height());
    if (left >= right || top >= bottom) {
        return; // The rect is fully clipped, no need to draw
    }

    if (shader && shader->setContext(fCTM)) {

        // If there is a shader, shade each row with the shader
        for (int y = top; y < bottom; ++y) {
            GPixel rowPixels[right - left];
            shader->shadeRow(left, y, right - left, rowPixels);
            GPixel* dstRow = fDevice.getAddr(left, y);
            for (int x = 0; x < right - left; ++x) {
                dstRow[x] = Blend(rowPixels[x], dstRow[x], mode);
            }
        }
    } else {
        // Fallback to solid color drawing if there is no shader
        GColor color = paint.getColor();
        GPixel srcPixel = GColorToPixel(color);

        for (int y = top; y < bottom; ++y) {
            GPixel* dstRow = fDevice.getAddr(left, y);
            for (int x = left; x < right; ++x) {
                dstRow[x - left] = Blend(srcPixel, dstRow[x - left], mode);
            }
        }
    }
}



void MyCanvas::drawConvexPolygon(const GPoint pts[], int count, const GPaint& paint) {
    if (count < 3) {
        return;  // A valid polygon must have at least 3 vertices
    }

    GShader* shader = paint.peekShader();
    GBlendMode mode = paint.getBlendMode();

    // First, transform all the points by the CTM
    GPoint transformedPts[count];
    fCTM.mapPoints(transformedPts, pts, count);
    
    // Calculate the bounding box of the transformed points
    float minX = transformedPts[0].x, maxX = transformedPts[0].x;
    float minY = transformedPts[0].y, maxY = transformedPts[0].y;

    for (int i = 1; i < count; ++i) {
        minX = std::min(minX, transformedPts[i].x);
        maxX = std::max(maxX, transformedPts[i].x);
        minY = std::min(minY, transformedPts[i].y);
        maxY = std::max(maxY, transformedPts[i].y);
    }

    // Clip the bounding box to the canvas size
    int left = std::max(GRoundToInt(minX), 0);
    int right = std::min(GRoundToInt(maxX), fDevice.width());
    int top = std::max(GRoundToInt(minY), 0);
    int bottom = std::min(GRoundToInt(maxY), fDevice.height());

    if (left >= right || top >= bottom) {
        return;  // The polygon is fully clipped, no need to draw
    }

    std::vector<float> intersections(count);

    for (int y = top; y < bottom; ++y) {
        int intersectionCount = 0;

        // Calculate all intersection points on the scanline
        for (int i = 0; i < count; ++i) {
            int next = (i + 1) % count;
            GPoint p0 = transformedPts[i];
            GPoint p1 = transformedPts[next];

            if ((p0.y <= y && p1.y > y) || (p1.y <= y && p0.y > y)) {
                float t = (y - p0.y) / (p1.y - p0.y);
                float x = p0.x + t * (p1.x - p0.x);
                intersections[intersectionCount++] = x;
            }
        }

        // Sort the intersection points
        std::sort(intersections.begin(), intersections.begin() + intersectionCount);

        // Fill the pixels between pairs of intersections
        for (int i = 0; i < intersectionCount; i += 2) {
            int startX = GRoundToInt(intersections[i]);
            int endX = GRoundToInt(intersections[i + 1]);
            startX = std::max(startX, left);
            endX = std::min(endX, right);

            if (startX >= endX) {
                continue;
            }

            GPixel* row = fDevice.getAddr(startX, y);
            if (shader && shader->setContext(fCTM)) {
                // Shader shading for the polygon
                GPixel rowPixels[endX - startX];
                shader->shadeRow(startX, y, endX - startX, rowPixels);
                for (int x = 0; x < endX - startX; ++x) {
                    row[x] = Blend(rowPixels[x], row[x], mode);
                }
            } else {
                // Fallback to solid color blending
                GColor color = paint.getColor();
                GPixel srcPixel = GColorToPixel(color);
                for (int x = startX; x < endX; ++x) {
                    row[x - startX] = Blend(srcPixel, row[x - startX], mode);
                }
            }
        }
    }
}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                        int count, const int indices[], const GPaint& paint) {
    for (int i = 0; i < count; ++i) {
        int index0 = indices[3 * i + 0];
        int index1 = indices[3 * i + 1];
        int index2 = indices[3 * i + 2];

        GPoint p0 = verts[index0];
        GPoint p1 = verts[index1];
        GPoint p2 = verts[index2];

        std::shared_ptr<GShader> shader;
        std::shared_ptr<GShader> shaderPtr(paint.peekShader(), [](GShader*) {});  // Wrap shaderPtr

        if (colors && !texs) {
            // Only color shader
            shader = std::make_shared<MyTriColorShader>(p0, p1, p2, colors[index0], colors[index1], colors[index2]);
        } else if (texs && !colors && shaderPtr) {
            // Only texture shader with ProxyShader for coordinate transformation
            GMatrix P = compute_basis(p0, p1, p2);
            GMatrix T = compute_basis(texs[index0], texs[index1], texs[index2]);
            auto invT = T.invert();  // Obtain the inverse matrix directly

            if (invT) {
                GMatrix textureToDrawing = GMatrix::Concat(P, *invT);
                shader = std::make_shared<ProxyShader>(shaderPtr, textureToDrawing);
            }
        } else if (colors && texs) {
            // Composite shader for both color and texture
            auto colorShader = std::make_shared<MyTriColorShader>(p0, p1, p2, colors[index0], colors[index1], colors[index2]);
            GMatrix P = compute_basis(p0, p1, p2);
            GMatrix T = compute_basis(texs[index0], texs[index1], texs[index2]);
            auto invT = T.invert();

            if (invT) {
                GMatrix textureToDrawing = GMatrix::Concat(P, *invT);
                shader = std::make_shared<CompositeShader>(
                    std::make_shared<ProxyShader>(shaderPtr, textureToDrawing), colorShader
                );
            }
        }

        
        GPaint trianglePaint = paint;
        if (shader) {
            trianglePaint.setShader(shader);
        }
        GPoint trianglePts[3] = {p0, p1, p2};
        drawConvexPolygon(trianglePts, 3, trianglePaint);
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                        int level, const GPaint& paint) {
    std::vector<GPoint> quadVerts;
    std::vector<GColor> quadColors;
    std::vector<GPoint> quadTexs;
    std::vector<int> indices;

    int gridSize = level + 1;
    float step = 1.0f / gridSize;

    // Generate vertices with bilinear interpolation
    for (int i = 0; i <= gridSize; ++i) {
        for (int j = 0; j <= gridSize; ++j) {
            float u = j * step;
            float v = i * step;

            GPoint vertex = verts[0] * (1 - u) * (1 - v) +
                            verts[1] * u * (1 - v) +
                            verts[2] * u * v +
                            verts[3] * (1 - u) * v;
            quadVerts.push_back(vertex);

            if (colors) {
                GColor color = colors[0] * (1 - u) * (1 - v) +
                               colors[1] * u * (1 - v) +
                               colors[2] * u * v +
                               colors[3] * (1 - u) * v;
                quadColors.push_back(color);
            }

            if (texs) {
                GPoint texCoord = texs[0] * (1 - u) * (1 - v) +
                                  texs[1] * u * (1 - v) +
                                  texs[2] * u * v +
                                  texs[3] * (1 - u) * v;
                quadTexs.push_back(texCoord);
            }
        }
    }

    // Generate indices for triangles in the grid
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            int idx0 = i * (gridSize + 1) + j;
            int idx1 = idx0 + 1;
            int idx2 = idx0 + gridSize + 1;
            int idx3 = idx2 + 1;

            indices.push_back(idx0);
            indices.push_back(idx1);
            indices.push_back(idx2);

            indices.push_back(idx1);
            indices.push_back(idx3);
            indices.push_back(idx2);
        }
    }

    // Call drawMesh with generated data
    drawMesh(quadVerts.data(),
             colors ? quadColors.data() : nullptr,
             texs ? quadTexs.data() : nullptr,
             indices.size() / 3,
             indices.data(),
             paint);
}


void MyCanvas::blit(int x, int y, int width, const GPaint& paint) {
    if (x < 0 || x >= fDevice.width()) {
        return;  // Out of bounds check
    }
    
    // Clip the width to the canvas bounds
    width = std::min(width, fDevice.width() - x);

    GPixel* row = fDevice.getAddr(x, y);

    GShader* shader = paint.peekShader();
    GBlendMode mode = paint.getBlendMode();  

    if (shader && shader->setContext(fCTM)) {
        // Use shader if available
        GPixel rowPixels[width];
        shader->shadeRow(x, y, width, rowPixels);

        // Blend each pixel using the shader
        for (int i = 0; i < width; ++i) {
            row[i] = Blend(rowPixels[i], row[i], mode);
        }
    } else {
        // No shader, use solid color
        GPixel srcPixel = GColorToPixel(paint.getColor());
        for (int i = 0; i < width; ++i) {
            row[i] = Blend(srcPixel, row[i], mode);
        }
    }
}

// Approximate quadratic and cubic curves using line segments with flattening
void MyCanvas::drawPath(const GPath& path, const GPaint& paint) {
    GPath::Edger edger(path);
    GPoint pts[4];
    std::vector<Edge> edges;
    const float tolerance = 0.25;  // 1/4 pixel tolerance
    int yMin = INT_MAX, yMax = INT_MIN;

    while (auto v = edger.next(pts)) {
        if (*v == GPathVerb::kLine) {
            addLineSegment(pts[0], pts[1], edges, yMin, yMax);
        } else if (*v == GPathVerb::kQuad) {
            flattenQuadratic(pts, edges, tolerance, yMin, yMax);
        } else if (*v == GPathVerb::kCubic) {
            flattenCubic(pts, edges, tolerance, yMin, yMax);
        }
    }

    yMin = std::max(0, yMin);
    yMax = std::min(fDevice.height(), yMax);

    renderEdges(edges, yMin, yMax, paint);
}

// Add a line segment to the edge list and update bounds
void MyCanvas::addLineSegment(GPoint p0, GPoint p1, std::vector<Edge>& edges, int& yMin, int& yMax) {
    fCTM.mapPoints(&p0, &p0, 1);
    fCTM.mapPoints(&p1, &p1, 1);

    Edge edge(p0, p1);
    if (edge.bottomY() != edge.topY()) {
        edges.push_back(edge);
        yMin = std::min(yMin, edge.topY());
        yMax = std::max(yMax, edge.bottomY());
    }
}

// Flatten a quadratic curve using recursive subdivision
void MyCanvas::flattenQuadratic(const GPoint pts[3], std::vector<Edge>& edges, float tolerance, int& yMin, int& yMax) {
    float dx = pts[2].x - pts[0].x;
    float dy = pts[2].y - pts[0].y;
    float d1x = pts[1].x - pts[0].x;
    float d1y = pts[1].y - pts[0].y;
    float error = std::abs(dx * d1y - dy * d1x) / sqrt(dx * dx + dy * dy);
    if (error <= tolerance) {
        addLineSegment(pts[0], pts[2], edges, yMin, yMax);
    } else {
        GPoint dst[5];
        GPath::ChopQuadAt(pts, dst, 0.5f);
        flattenQuadratic(dst, edges, tolerance, yMin, yMax);
        flattenQuadratic(dst + 2, edges, tolerance, yMin, yMax);
    }
}

// Flatten a cubic curve using recursive subdivision
void MyCanvas::flattenCubic(const GPoint pts[4], std::vector<Edge>& edges, float tolerance, int& yMin, int& yMax) {
    float dx = pts[3].x - pts[0].x;
    float dy = pts[3].y - pts[0].y;
    float d1x = pts[1].x - pts[0].x;
    float d1y = pts[1].y - pts[0].y;
    float d2x = pts[2].x - pts[1].x;
    float d2y = pts[2].y - pts[1].y;

    float error = std::abs(dy * d1x - dx * d1y) + std::abs(dy * d2x - dx * d2y);
    error /= sqrt(dx * dx + dy * dy);

    if (error <= tolerance) {
        addLineSegment(pts[0], pts[3], edges, yMin, yMax);
    } else {
        GPoint dst[7];
        GPath::ChopCubicAt(pts, dst, 0.5f);
        flattenCubic(dst, edges, tolerance, yMin, yMax);
        flattenCubic(dst + 3, edges, tolerance, yMin, yMax);
    }
}

// Render edges to fill the path using scanline
void MyCanvas::renderEdges(std::vector<Edge>& edges, int yMin, int yMax, const GPaint& paint) {
    for (int y = yMin; y < yMax; ++y) {
        if (y < 0 || y >= fDevice.height()) continue;

        int winding = 0;
        int L = 0;
        std::vector<Edge*> activeEdges;

        for (auto& edge : edges) {
            if (edge.isValid(y)) {
                activeEdges.push_back(&edge);
            }
        }

        std::sort(activeEdges.begin(), activeEdges.end(), [y](const Edge* e1, const Edge* e2) {
            return e1->computeX(y) < e2->computeX(y);
        });

        for (size_t i = 0; i < activeEdges.size(); ++i) {
            int x = GRoundToInt(activeEdges[i]->computeX(y));
            if (winding == 0) {
                L = x;
            }
            winding += activeEdges[i]->windingValue();

            if (winding == 0) {
                int R = x;
                if (L > R) std::swap(L, R);
                L = std::max(0, L);
                R = std::min(fDevice.width(), R);

                if (L < R) {
                    blit(L, y, R - L, paint);
                }
            }
        }
    }
}


// Define the GCreateCanvas function to return an instance of MyCanvas
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& bitmap) {
    return std::make_unique<MyCanvas>(bitmap);
}