#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform sampler2D FillASCII;
uniform float _Exposure;
uniform float _Attenuation;
uniform bool _InvertLuminance;
uniform vec3 _ASCIIColor;
uniform vec3 _BackgroundColor;
uniform float _BlendWithBase;

float luminance(vec3 rgb) {
    return max(0.00001, dot(rgb, vec3(0.2127, 0.7152, 0.0722)));
}

void main()
{
    vec3 color = texture(inputTexture, TexCoord).rgb;
    float lum = luminance(color);
    lum = pow(lum * _Exposure, _Attenuation);
    if (_InvertLuminance) {
        lum = 1.0 - lum;
    }

    int fillIndex = int(lum * 10.0);
    vec2 fillUV = vec2(float(fillIndex) / 10.0 + fract(TexCoord.x * 8.0) / 80.0, 1.0 - fract(TexCoord.y * 8.0));
    vec3 ascii = texture(FillASCII, fillUV).rgb;

    vec3 asciiColor = mix(_BackgroundColor, _ASCIIColor, ascii.r);
    vec3 finalColor = mix(asciiColor, color, _BlendWithBase);

    FragColor = vec4(finalColor, 1.0);
}