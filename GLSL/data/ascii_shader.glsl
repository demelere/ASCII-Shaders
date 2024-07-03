#version 330

uniform sampler2D texture;
uniform float _Zoom = 1.0;
uniform vec2 _Offset = vec2(0.0, 0.0);
uniform int _KernelSize = 2;
uniform float _Sigma = 2.0;
uniform float _SigmaScale = 1.6;
uniform float _Tau = 1.0;
uniform float _Threshold = 0.005;
uniform bool _UseDepth = true;
uniform float _DepthThreshold = 0.1;
uniform bool _UseNormals = true;
uniform float _NormalThreshold = 0.1;
uniform float _DepthCutoff = 0.0;
uniform int _EdgeThreshold = 8;
uniform bool _Edges = true;
uniform bool _Fill = true;
uniform float _Exposure = 1.0;
uniform float _Attenuation = 1.0;
uniform bool _InvertLuminance = false;
uniform vec3 _ASCIIColor = vec3(1.0, 1.0, 1.0);
uniform vec3 _BackgroundColor = vec3(0.0, 0.0, 0.0);
uniform float _BlendWithBase = 0.0;
uniform float _DepthFalloff = 0.0;
uniform float _DepthOffset = 0.0;

uniform sampler2D EdgesASCII;
uniform sampler2D FillASCII;
uniform sampler2D Normals;

in vec2 texCoord;
out vec4 fragColor;

const float PI = 3.14159265358979323846;

float gaussian(float sigma, float pos) {
    return (1.0 / sqrt(2.0 * PI * sigma * sigma)) * exp(-(pos * pos) / (2.0 * sigma * sigma));
}

vec2 transformUV(vec2 uv) {
    vec2 zoomUV = uv * 2.0 - 1.0;
    zoomUV += vec2(-_Offset.x, _Offset.y) * 2.0;
    zoomUV *= _Zoom;
    zoomUV = zoomUV * 0.5 + 0.5;
    return zoomUV;
}

float luminance(vec3 color) {
    return max(0.00001, dot(color, vec3(0.2127, 0.7152, 0.0722)));
}

void main() {
    vec2 uv = transformUV(texCoord);
    vec3 color = texture(texture, uv).rgb;
    float depth = texture(Normals, uv).a;
    vec3 normal = texture(Normals, uv).rgb * 2.0 - 1.0;

    // Main shader logic:
    // 1. Calculating luminance
    // 2. Applying Gaussian blur
    // 3. Calculating Difference of Gaussians
    // 4. Edge detection
    // 5. ASCII character selection and rendering
    // Final color output

    // Implement Gaussian blur and edge detection
    float blur1 = 0.0;
    float blur2 = 0.0;
    float kernelSum1 = 0.0;
    float kernelSum2 = 0.0;

    for (int x = -_KernelSize; x <= _KernelSize; ++x) {
        for (int y = -_KernelSize; y <= _KernelSize; ++y) {
            vec2 offset = vec2(x, y) / textureSize(texture, 0);
            float lum = luminance(texture(texture, uv + offset).rgb);
            float gauss1 = gaussian(_Sigma, length(vec2(x, y)));
            float gauss2 = gaussian(_Sigma * _SigmaScale, length(vec2(x, y)));
            
            blur1 += lum * gauss1;
            blur2 += lum * gauss2;
            kernelSum1 += gauss1;
            kernelSum2 += gauss2;
        }
    }

    blur1 /= kernelSum1;
    blur2 /= kernelSum2;

    float edge = (blur1 - _Tau * blur2) >= _Threshold ? 1.0 : 0.0;

    if (_UseDepth || _UseNormals) {
        float depthEdge = 0.0;
        float normalEdge = 0.0;

        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                if (x == 0 && y == 0) continue;
                vec2 offset = vec2(x, y) / textureSize(texture, 0);
                float neighborDepth = texture(Normals, uv + offset).a;
                vec3 neighborNormal = texture(Normals, uv + offset).rgb * 2.0 - 1.0;

                if (_UseDepth && abs(neighborDepth - depth) > _DepthThreshold) {
                    depthEdge = 1.0;
                }

                if (_UseNormals && distance(neighborNormal, normal) > _NormalThreshold) {
                    normalEdge = 1.0;
                }
            }
        }

        edge = max(edge, max(depthEdge, normalEdge));
    }

    if (_DepthCutoff > 0.0 && depth * 1000.0 > _DepthCutoff) {
        edge = 0.0;
    }

    // ASCII character selection
    vec3 ascii = vec3(0.0);
    float lum = luminance(color);
    lum = pow(lum * _Exposure, _Attenuation);
    if (_InvertLuminance) lum = 1.0 - lum;

    if (_Edges && edge > 0.0) {
        int edgeIndex = int(edge * 4.0);
        vec2 edgeUV = vec2(float(edgeIndex) / 4.0 + fract(uv.x * 8.0) / 32.0, 1.0 - fract(uv.y * 8.0));
        ascii = texture(EdgesASCII, edgeUV).rgb;
    } else if (_Fill) {
        int fillIndex = int(lum * 10.0);
        vec2 fillUV = vec2(float(fillIndex) / 10.0 + fract(uv.x * 8.0) / 80.0, 1.0 - fract(uv.y * 8.0));
        ascii = texture(FillASCII, fillUV).rgb;
    }

    // Apply colors and blending
    vec3 asciiColor = mix(_BackgroundColor, _ASCIIColor, ascii.r);
    vec3 finalColor = mix(asciiColor, color, _BlendWithBase);

    // Apply depth falloff
    float depthFactor = 1.0 - smoothstep(_DepthOffset, _DepthOffset + _DepthFalloff, depth);
    finalColor = mix(_BackgroundColor, finalColor, depthFactor);

    fragColor = vec4(finalColor, 1.0);
}