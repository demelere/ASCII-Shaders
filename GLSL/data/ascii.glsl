#version 150

uniform sampler2D texture;
uniform sampler2D EdgesASCII;
uniform sampler2D FillASCII;

uniform float _Exposure = 1.0;
uniform float _Attenuation = 1.0;
uniform bool _InvertLuminance = false;
uniform vec3 _ASCIIColor = vec3(1.0, 1.0, 1.0);
uniform vec3 _BackgroundColor = vec3(0.0, 0.0, 0.0);
uniform float _BlendWithBase = 0.0;

in vec4 vertTexCoord;
out vec4 fragColor;

float luminance(vec3 color) {
    return max(0.00001, dot(color, vec3(0.2127, 0.7152, 0.0722)));
}

void main() {
    vec2 uv = vertTexCoord.st;
    vec3 color = texture2D(texture, uv).rgb;
    
    float lum = luminance(color);
    lum = pow(lum * _Exposure, _Attenuation);
    if (_InvertLuminance) lum = 1.0 - lum;

    int fillIndex = int(lum * 10.0);
    vec2 fillUV = vec2(float(fillIndex) / 10.0 + fract(uv.x * 8.0) / 80.0, 1.0 - fract(uv.y * 8.0));
    vec3 ascii = texture2D(FillASCII, fillUV).rgb;

    vec3 asciiColor = mix(_BackgroundColor, _ASCIIColor, ascii.r);
    vec3 finalColor = mix(asciiColor, color, _BlendWithBase);

    fragColor = vec4(finalColor, 1.0);
}