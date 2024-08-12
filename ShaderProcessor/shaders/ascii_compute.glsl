#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;

layout(rgba32f, binding = 0) uniform image2D inputImage;
layout(rgba32f, binding = 1) uniform image2D outputImage;

uniform sampler2D EdgesASCII;
uniform sampler2D FillASCII;
uniform sampler2D Sobel;
uniform sampler2D Downscale;

uniform int _EdgeThreshold;
uniform bool _Edges;
uniform bool _Fill;
uniform float _Exposure;
uniform float _Attenuation;
uniform bool _InvertLuminance;
uniform vec3 _ASCIIColor;
uniform vec3 _BackgroundColor;
uniform float _BlendWithBase;

shared int edgeCount[64];

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(inputImage);
    
    if (pixelCoords.x >= imageSize.x || pixelCoords.y >= imageSize.y) {
        return;
    }

    vec2 uv = vec2(pixelCoords) / vec2(imageSize);
    vec2 sobel = texelFetch(Sobel, pixelCoords, 0).rg;
    float theta = sobel.r;
    float absTheta = abs(theta) / 3.14159265358979323846;

    int direction = -1;
    if (absTheta < 0.125 || absTheta > 0.875) direction = 0;
    else if (absTheta < 0.375) direction = 1;
    else if (absTheta < 0.625) direction = 2;
    else direction = 3;

    if (gl_LocalInvocationIndex == 0) {
        for (int i = 0; i < 64; i++) {
            edgeCount[i] = 0;
        }
    }
    barrier();

    atomicAdd(edgeCount[direction + 1], 1);
    barrier();

    int commonEdgeIndex = -1;
    if (gl_LocalInvocationIndex == 0) {
        int maxValue = 0;
        for (int i = 0; i < 4; i++) {
            if (edgeCount[i] > maxValue) {
                maxValue = edgeCount[i];
                commonEdgeIndex = i - 1;
            }
        }
        if (maxValue < _EdgeThreshold) commonEdgeIndex = -1;
        edgeCount[0] = commonEdgeIndex;
    }
    barrier();

    commonEdgeIndex = edgeCount[0];

    vec3 ascii = vec3(0.0);
    ivec2 downscaleID = pixelCoords / 8;
    vec4 downscaleInfo = texelFetch(Downscale, downscaleID, 0);

    if (commonEdgeIndex >= 0 && _Edges) {
        vec2 localUV;
        localUV.x = (float(pixelCoords.x % 8) + float(commonEdgeIndex + 1) * 8.0) / 32.0;
        localUV.y = 1.0 - float(pixelCoords.y % 8) / 8.0;
        ascii = texture(EdgesASCII, localUV).rgb;
    } else if (_Fill) {
        float luminance = clamp(pow(downscaleInfo.w * _Exposure, _Attenuation), 0.0, 1.0);
        if (_InvertLuminance) luminance = 1.0 - luminance;
        luminance = max(0.0, (floor(luminance * 10.0) - 1.0)) / 10.0;
        
        vec2 localUV;
        localUV.x = (float(pixelCoords.x % 8) + luminance * 80.0) / 80.0;
        localUV.y = float(pixelCoords.y % 8) / 8.0;
        ascii = texture(FillASCII, localUV).rgb;
    }

    ascii = mix(_BackgroundColor, mix(_ASCIIColor, downscaleInfo.rgb, _BlendWithBase), ascii.r);

    imageStore(outputImage, pixelCoords, vec4(ascii, 1.0));
}