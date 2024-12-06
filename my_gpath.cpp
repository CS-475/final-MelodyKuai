#include "my_gpath.h"

void GPathBuilder::addRect(const GRect& r, GPathDirection dir) {
    if (dir == GPathDirection::kCW) {  // Clockwise direction
        moveTo(r.left, r.top);
        lineTo(r.right, r.top);
        lineTo(r.right, r.bottom);
        lineTo(r.left, r.bottom);
    } else {  // Counterclockwise direction
        moveTo(r.left, r.top);
        lineTo(r.left, r.bottom);
        lineTo(r.right, r.bottom);
        lineTo(r.right, r.top);
    }
    // Close the path by connecting the last point to the starting point
    lineTo(r.left, r.top);
}

void GPathBuilder::addPolygon(const GPoint pts[], int count) {
    if (count < 0) return;  

    moveTo(pts[0]); 
    for (int i = 1; i < count; ++i) {
        lineTo(pts[i]);  
    }
}

const float kCtrlPoint = 0.41421356f; // std::tan(M_PI / 8.0f)

void GPathBuilder::addCircle(GPoint center, float radius, GPathDirection dir) {
    // Define 16 points (alternating main points and control points) to approximate a circle
    const GPoint unitCirclePts[16] = {
        {1, 0},
        {1, kCtrlPoint},
        {std::sqrt(2.0f) / 2, std::sqrt(2.0f) / 2},
        {kCtrlPoint, 1},
        {0, 1},
        {-kCtrlPoint, 1},
        {-std::sqrt(2.0f) / 2, std::sqrt(2.0f) / 2},
        {-1, kCtrlPoint},
        {-1, 0},
        {-1, -kCtrlPoint},
        {-std::sqrt(2.0f) / 2, -std::sqrt(2.0f) / 2},
        {-kCtrlPoint, -1},
        {0, -1},
        {kCtrlPoint, -1},
        {std::sqrt(2.0f) / 2, -std::sqrt(2.0f) / 2},
        {1, -kCtrlPoint}
    };

    // Create a scaling and translation matrix to transform unit circle points to the specified center and radius
    GMatrix transform = GMatrix::Translate(center.x, center.y) * GMatrix::Scale(radius, radius);
    GPoint pts[16];
    transform.mapPoints(pts, unitCirclePts, 16);

    // Set the starting point of the path
    moveTo(pts[0]);

    // Draw the full circle in clockwise or counterclockwise order, adding a 22.5° arc for each segment
    if (dir == GPathDirection::kCW) {
        for (int i = 0; i < 16; i += 2) {
            quadTo(pts[i + 1], pts[(i + 2) % 16]);  
        }
    } else {
        for (int i = 16; i > 0; i -= 2) {
            quadTo(pts[i - 1], pts[i - 2]);  
        }
    }
}

// ChopQuadAt implementation
void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    GPoint ab = { (1 - t) * src[0].x + t * src[1].x, (1 - t) * src[0].y + t * src[1].y };
    GPoint bc = { (1 - t) * src[1].x + t * src[2].x, (1 - t) * src[1].y + t * src[2].y };
    GPoint abc = { (1 - t) * ab.x + t * bc.x, (1 - t) * ab.y + t * bc.y };

    dst[0] = src[0];
    dst[1] = ab;
    dst[2] = abc;
    dst[3] = bc;
    dst[4] = src[2];
}


// ChopCubicAt implementation
void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    GPoint ab = { (1 - t) * src[0].x + t * src[1].x, (1 - t) * src[0].y + t * src[1].y };
    GPoint bc = { (1 - t) * src[1].x + t * src[2].x, (1 - t) * src[1].y + t * src[2].y };
    GPoint cd = { (1 - t) * src[2].x + t * src[3].x, (1 - t) * src[2].y + t * src[3].y };
    GPoint abc = { (1 - t) * ab.x + t * bc.x, (1 - t) * ab.y + t * bc.y };
    GPoint bcd = { (1 - t) * bc.x + t * cd.x, (1 - t) * bc.y + t * cd.y };
    GPoint abcd = { (1 - t) * abc.x + t * bcd.x, (1 - t) * abc.y + t * bcd.y };

    dst[0] = src[0];
    dst[1] = ab;
    dst[2] = abc;
    dst[3] = abcd;
    dst[4] = bcd;
    dst[5] = cd;
    dst[6] = src[3];
}


GRect GPath::bounds() const {
    if (fPts.empty()) {
        return GRect::LTRB(0, 0, 0, 0); // Return zero rectangle for empty path
    }

    float minX = fPts[0].x, maxX = fPts[0].x;
    float minY = fPts[0].y, maxY = fPts[0].y;

    GPoint pts[GPath::kMaxNextPoints];
    Iter iter(*this);
    
    // Iterate through each segment of the path
    while (auto v = iter.next(pts)) {
        switch (v.value()) {
            case kLine: {
                // Compute bounds for line segment
                minX = std::min(minX, std::min(pts[0].x, pts[1].x));
                maxX = std::max(maxX, std::max(pts[0].x, pts[1].x));
                minY = std::min(minY, std::min(pts[0].y, pts[1].y));
                maxY = std::max(maxY, std::max(pts[0].y, pts[1].y));
                break;
            }
            case kQuad: {
                computeQuadBounds(pts, minX, maxX, minY, maxY); // Bounds for quadratic curve
                break;
            }
            case kCubic: {
                computeCubicBounds(pts, minX, maxX, minY, maxY); // Bounds for cubic curve
                break;
            }
            default:
                break;
        }
    }

    return GRect::LTRB(minX, minY, maxX, maxY); // Return tight bounds of the path
}

void computeQuadBounds(const GPoint pts[3], float& minX, float& maxX, float& minY, float& maxY) {
    minX = std::min(minX, std::min(pts[0].x, pts[2].x));
    maxX = std::max(maxX, std::max(pts[0].x, pts[2].x));
    minY = std::min(minY, std::min(pts[0].y, pts[2].y));
    maxY = std::max(maxY, std::max(pts[0].y, pts[2].y));

    float tx = (pts[0].x - pts[1].x) / (pts[0].x - 2 * pts[1].x + pts[2].x);
    float ty = (pts[0].y - pts[1].y) / (pts[0].y - 2 * pts[1].y + pts[2].y);

    if (tx > 0 && tx < 1) {
        float x = (1 - tx) * (1 - tx) * pts[0].x + 2 * (1 - tx) * tx * pts[1].x + tx * tx * pts[2].x;
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
    }
    if (ty > 0 && ty < 1) {
        float y = (1 - ty) * (1 - ty) * pts[0].y + 2 * (1 - ty) * ty * pts[1].y + ty * ty * pts[2].y;
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }
}

void computeCubicBounds(const GPoint pts[4], float& minX, float& maxX, float& minY, float& maxY) {
    minX = std::min(minX, std::min(pts[0].x, pts[3].x));
    maxX = std::max(maxX, std::max(pts[0].x, pts[3].x));
    minY = std::min(minY, std::min(pts[0].y, pts[3].y));
    maxY = std::max(maxY, std::max(pts[0].y, pts[3].y));

    float aX = -3 * pts[0].x + 9 * pts[1].x - 9 * pts[2].x + 3 * pts[3].x;
    float bX = 6 * pts[0].x - 12 * pts[1].x + 6 * pts[2].x;
    float cX = -3 * pts[0].x + 3 * pts[1].x;

    float aY = -3 * pts[0].y + 9 * pts[1].y - 9 * pts[2].y + 3 * pts[3].y;
    float bY = 6 * pts[0].y - 12 * pts[1].y + 6 * pts[2].y;
    float cY = -3 * pts[0].y + 3 * pts[1].y;

    float tx[2], ty[2];
    int countX = solveQuadratic(aX, bX, cX, tx);
    int countY = solveQuadratic(aY, bY, cY, ty);
    
    for (int i = 0; i < countX; i++) {
        if (tx[i] > 0 && tx[i] < 1) {
            float x = cubicAt(pts[0].x, pts[1].x, pts[2].x, pts[3].x, tx[i]);
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
        }
    }

    for (int i = 0; i < countY; i++) {
        if (ty[i] > 0 && ty[i] < 1) {
            float y = cubicAt(pts[0].y, pts[1].y, pts[2].y, pts[3].y, ty[i]);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }
    }
}

float cubicAt(float A, float B, float C, float D, float t) {
    return A * (1 - t) * (1 - t) * (1 - t) +
           3 * B * (1 - t) * (1 - t) * t +
           3 * C * (1 - t) * t * t +
           D * t * t * t;
}

int solveQuadratic(float a, float b, float c, float roots[2]) {
    if (a == 0) {
        if (b == 0) {
            return (c == 0) ? -1 : 0;
        }
        roots[0] = -c / b;
        return 1;
    }

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) return 0;

    float sqrtD = sqrtf(discriminant);
    if (discriminant == 0) {
        roots[0] = -b / (2 * a);
        return 1;
    } else {
        roots[0] = (-b + sqrtD) / (2 * a);
        roots[1] = (-b - sqrtD) / (2 * a);
        return 2;
    }
}