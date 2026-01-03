#version 410 core

out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;

void main()
{
    fragColor.xy = gl_FragCoord.xy / (iResolution * iTime);
}
