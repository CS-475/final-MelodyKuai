#include "bitmap_shader.h"

BitmapShader::BitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode)
    : fBitmap(bitmap), fLocalMatrix(localMatrix), fTileMode(tileMode) {
    fOpaque = bitmap.isOpaque();
}

bool BitmapShader::isOpaque() {
    return fOpaque;
}

bool BitmapShader::setContext(const GMatrix& ctm) {
    GMatrix totalMatrix = GMatrix::Concat(ctm, fLocalMatrix);
    auto invMatrix = totalMatrix.invert();
    if (!invMatrix) {
        return false;
    }
    fInverse = *invMatrix;
    return true;
}

void BitmapShader::shadeRow(int x, int y, int count, GPixel row[]) {
    for (int i = 0; i < count; ++i) {
        GPoint srcPoint = { x + i + 0.5f, y + 0.5f };
        fInverse.mapPoints(&srcPoint, &srcPoint, 1);

        int bitmapX, bitmapY;
        switch (fTileMode) {
            case GTileMode::kClamp:
                bitmapX = std::clamp(static_cast<int>(std::floor(srcPoint.x)), 0, fBitmap.width() - 1);
                bitmapY = std::clamp(static_cast<int>(std::floor(srcPoint.y)), 0, fBitmap.height() - 1);
                break;

            case GTileMode::kRepeat:
                bitmapX = static_cast<int>(std::floor(srcPoint.x)) % fBitmap.width();
                if (bitmapX < 0) bitmapX += fBitmap.width(); 
                bitmapY = static_cast<int>(std::floor(srcPoint.y)) % fBitmap.height();
                if (bitmapY < 0) bitmapY += fBitmap.height(); 
                break;

            case GTileMode::kMirror:
                bitmapX = static_cast<int>(std::floor(srcPoint.x));
                bitmapY = static_cast<int>(std::floor(srcPoint.y));
                
                bitmapX = bitmapX % (2 * fBitmap.width());
                if (bitmapX < 0) bitmapX += 2 * fBitmap.width();
                if (bitmapX >= fBitmap.width()) bitmapX = 2 * fBitmap.width() - bitmapX - 1;

                bitmapY = bitmapY % (2 * fBitmap.height());
                if (bitmapY < 0) bitmapY += 2 * fBitmap.height();
                if (bitmapY >= fBitmap.height()) bitmapY = 2 * fBitmap.height() - bitmapY - 1;
                break;
        }

        if (bitmapX >= 0 && bitmapX < fBitmap.width() && bitmapY >= 0 && bitmapY < fBitmap.height()) {
            row[i] = *fBitmap.getAddr(bitmapX, bitmapY);
        } else {
            row[i] = GPixel_PackARGB(0, 0, 0, 0);  
        }
    }
}


std::shared_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode) {
    return std::make_shared<BitmapShader>(bitmap, localMatrix, tileMode);
}