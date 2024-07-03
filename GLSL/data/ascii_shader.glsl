#version 150

uniform sampler2D texture;
uniform float time;
uniform vec2 resolution;

uniform float _Zoom = 1.0;
uniform vec2 _Offset = vec2(0.0);
uniform float _Exposure = 1.0;
uniform float _Attenuation = 1.0;
uniform bool _InvertLuminance = false;
uniform vec3 _ASCIIColor = vec3(1.0);
uniform vec3 _BackgroundColor = vec3(0.0);
uniform float _BlendWithBase = 0.0;

in vec2 texCoordVarying;
out vec4 fragColor;

const vec2 ASCII_CHARS = vec2(80.0, 8.0);

float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec2 transformUV(vec2 uv) {
    vec2 zoomUV = uv * 2.0 - 1.0;
    zoomUV += -_Offset * 2.0;
    zoomUV *= _Zoom;
    return zoomUV * 0.5 + 0.5;
}

void main() {
    vec2 uv = transformUV(texCoordVarying);
    vec4 color = texture2D(texture, uv);
    
    float lum = luminance(color.rgb);
    lum = pow(lum * _Exposure, _Attenuation);
    
    if (_InvertLuminance) lum = 1.0 - lum;
    
    vec2 ascii_uv = fract(gl_FragCoord.xy / ASCII_CHARS);
    float ascii_lum = floor(lum * 10.0) / 10.0;
    
    vec2 ascii_tex_coord = vec2(ascii_uv.x + ascii_lum, ascii_uv.y);
    float ascii = texture2D(texture, ascii_tex_coord).r;
    
    vec3 final_color = mix(_BackgroundColor, mix(_ASCIIColor, color.rgb, _BlendWithBase), ascii);
    
    fragColor = vec4(final_color, 1.0);
}