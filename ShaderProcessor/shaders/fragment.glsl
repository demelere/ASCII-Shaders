#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform sampler2D FillASCII;
uniform sampler2D EdgesASCII;

uniform float _Zoom;
uniform vec2 _Offset;
uniform int _KernelSize;
uniform float _Sigma;
uniform float _SigmaScale;
uniform float _Tau;
uniform float _Threshold;
uniform bool _UseDepth;
uniform float _DepthThreshold;
uniform bool _UseNormals;
uniform float _NormalThreshold;
uniform float _DepthCutoff;
uniform int _EdgeThreshold;
uniform bool _Edges;
uniform bool _Fill;
uniform float _Exposure;
uniform float _Attenuation;
uniform bool _InvertLuminance;
uniform vec3 _ASCIIColor;
uniform vec3 _BackgroundColor;
uniform float _BlendWithBase;
uniform float _DepthFalloff;
uniform float _DepthOffset;

float luminance(vec3 rgb) {
    return max(0.00001, dot(rgb, vec3(0.2127, 0.7152, 0.0722)));
}

float gaussian(float sigma, float pos) {
    return (1.0 / sqrt(2.0 * 3.14159265358979323846 * sigma * sigma)) * exp(-(pos * pos) / (2.0 * sigma * sigma));
}

vec2 transformUV(vec2 uv) {
    vec2 zoomUV = uv * 2.0 - 1.0;
    zoomUV += vec2(-_Offset.x, _Offset.y) * 2.0;
    zoomUV *= _Zoom;
    zoomUV = zoomUV * 0.5 + 0.5;
    return zoomUV;
}

void main()
{
    vec2 uv = transformUV(TexCoord);
    vec3 color = texture(inputTexture, uv).rgb;
    
    // Implement Gaussian blur and edge detection
    float blur1 = 0.0;
    float blur2 = 0.0;
    float kernelSum1 = 0.0;
    float kernelSum2 = 0.0;

    for (int x = -_KernelSize; x <= _KernelSize; ++x) {
        for (int y = -_KernelSize; y <= _KernelSize; ++y) {
            vec2 offset = vec2(float(x), float(y)) / textureSize(inputTexture, 0);
            float lum = luminance(texture(inputTexture, uv + offset).rgb);
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

    // Simplified depth and normal checks (since we don't have actual depth and normal textures)
    // In a real implementation, you'd sample from depth and normal textures here

    if (_DepthCutoff > 0.0) {
        // Simulating depth cutoff
        float simDepth = (uv.x + uv.y) * 0.5; // This is just a placeholder
        if (simDepth * 1000.0 > _DepthCutoff) {
            edge = 0.0;
        }
    }

    // ASCII character selection
    vec3 ascii = vec3(0.0);
    float lum = luminance(color);
    lum = pow(lum * _Exposure, _Attenuation);
    if (_InvertLuminance) {
        lum = 1.0 - lum;
    }

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

    // Apply depth falloff (simulated)
    float simDepth = (uv.x + uv.y) * 0.5; // This is just a placeholder
    float depthFactor = 1.0 - smoothstep(_DepthOffset, _DepthOffset + _DepthFalloff, simDepth);
    finalColor = mix(_BackgroundColor, finalColor, depthFactor);

    FragColor = vec4(finalColor, 1.0);
}