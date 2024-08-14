#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform sampler2D FillASCII;
uniform sampler2D EdgesASCII;
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
uniform float _DepthFalloff;
uniform float _DepthOffset;

float luminance(vec3 rgb) {
    return max(0.00001, dot(rgb, vec3(0.2127, 0.7152, 0.0722)));
}

void main() {
    vec2 uv = TexCoord;
    vec4 color = texture(inputTexture, uv);
    vec2 sobel = texture(Sobel, uv).rg;
    float theta = sobel.r;
    float absTheta = abs(theta) / 3.14159265358979323846;

    int direction = -1;
    if (absTheta < 0.125 || absTheta > 0.875) direction = 0;
    else if (absTheta < 0.375) direction = 1;
    else if (absTheta < 0.625) direction = 2;
    else direction = 3;

    vec3 ascii = vec3(0.0);
    vec4 downscaleInfo = texture(Downscale, uv);

    if (direction >= 0 && _Edges) {
        vec2 edgeUV;
        edgeUV.x = (float(int(uv.x * 8.0) % 8) + float(direction + 1) * 8.0) / 32.0;
        edgeUV.y = 1.0 - float(int(uv.y * 8.0) % 8) / 8.0;
        ascii = texture(EdgesASCII, edgeUV).rgb;
    } else if (_Fill) {
        float luminance = clamp(pow(downscaleInfo.w * _Exposure, _Attenuation), 0.0, 1.0);
        if (_InvertLuminance) luminance = 1.0 - luminance;
        luminance = max(0.0, (floor(luminance * 10.0) - 1.0)) / 10.0;
        
        vec2 fillUV;
        fillUV.x = (float(int(uv.x * 8.0) % 8) + luminance * 80.0) / 80.0;
        fillUV.y = float(int(uv.y * 8.0) % 8) / 8.0;
        ascii = texture(FillASCII, fillUV).rgb;
    }

    ascii = mix(_BackgroundColor, mix(_ASCIIColor, downscaleInfo.rgb, _BlendWithBase), ascii.r);

    // Apply depth falloff (simulated)
    float simDepth = (uv.x + uv.y) * 0.5; // This is just a placeholder
    float depthFactor = 1.0 - smoothstep(_DepthOffset, _DepthOffset + _DepthFalloff, simDepth);
    vec3 finalColor = mix(_BackgroundColor, ascii, depthFactor);

    FragColor = vec4(finalColor, 1.0);
}