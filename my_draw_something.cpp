#include "./include/GCanvas.h"
#include "./include/GRect.h"
#include "./include/GColor.h"
#include "./include/GPoint.h"
#include "./include/GBitmap.h"
#include "./include/GShader.h"
#include "./include/GPath.h"
#include "./include/GPathBuilder.h"
#include "composite_shader.h"
#include <cmath>
#include <memory>

// Function to create a wave-like path
void createWavePath(GPathBuilder& builder, int waves, float cx, float cy, float radius, float amplitude) {
    float angleStep = static_cast<float>(M_PI) / waves;
    for (int i = 0; i <= 2 * waves; ++i) {
        float angle = i * angleStep;
        float offsetRadius = radius + amplitude * std::sin(angle * 3);  // Adds a wave effect
        GPoint pt = {cx + std::cos(angle) * offsetRadius, cy + std::sin(angle) * offsetRadius};
        if (i == 0) {
            builder.moveTo(pt);
        } else {
            builder.lineTo(pt);
        }
    }
}

// Create a linear gradient shader within the wave's bounding box
std::shared_ptr<GShader> createAdjustedGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count) {
    return GCreateLinearGradient(p0, p1, colors, count);
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    canvas->clear(GColor::RGB(1, 1, 1));

    int waves = 15;
    float cx = dim.width * 0.5f;  // Center x
    float cy = dim.height * 0.6f;  // Center y
    float radius = 100.0f;  // Radius of the wave pattern
    float amplitude = 20.0f;  // Controls wave height
    
    // Define a more harmonious gradient color scheme
    GColor colors[] = {
        GColor::RGBA(1.0f, 0.3f, 0, 1.0f),  
        GColor::RGBA(0, 1.0f, 0.3f, 1.0f),  
        GColor::RGBA(0.3f, 0, 1.0f, 1.0f)   
    };

    // Create wave path
    GPathBuilder builder;
    createWavePath(builder, waves, cx, cy, radius, amplitude);
    std::shared_ptr<GPath> wavePath = builder.detach();

    // Define a gradient shader for the wave
    float gradientRadius = radius * 1.2f;
    GPoint p0 = {cx - gradientRadius, cy - gradientRadius};
    GPoint p1 = {cx + gradientRadius, cy + gradientRadius};
    std::shared_ptr<GShader> gradientShader = createAdjustedGradientShader(p0, p1, colors, 3);

    // Load a texture to combine with the gradient
    GBitmap textureBitmap;
    textureBitmap.readFromFile("./apps/wood1.png");  // Replace with actual texture path
    GMatrix textureMatrix = GMatrix::Scale(1.0f / textureBitmap.width(), 1.0f / textureBitmap.height());
    std::shared_ptr<GShader> textureShader = GCreateBitmapShader(textureBitmap, textureMatrix, GTileMode::kRepeat);

    // Combine the gradient and texture into a composite shader
    std::shared_ptr<GShader> compositeShader = std::make_shared<CompositeShader>(gradientShader, textureShader);
    
    // Set up paint with the composite shader
    GPaint paint;
    paint.setShader(compositeShader);

    // Draw the wave path with the composite shader applied
    canvas->drawPath(*wavePath, paint);

    return "Composite Gradient and Texture Shader";
}
