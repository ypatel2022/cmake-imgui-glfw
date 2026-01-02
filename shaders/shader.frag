#version 410 core

out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution;
    vec2 p = gl_FragCoord.xy - 0.5 * iResolution;
    bool in_range = length(p) < (int(iTime * 500) % 1000);

    if (in_range) {
        fragColor = vec4(uv, 1.0, 1.0);
    } else {
        fragColor = vec4(0.0);
    }
}
