R"GLSL(#version 300 es
precision mediump float;
precision mediump int;

uniform sampler2D uTex0;
uniform sampler2D uTex1;
uniform int   uUseTexture;
uniform int   uUseLightmap;
uniform float uAlphaRef;
uniform vec4  uFogColor;
uniform int   uFogEnable;
uniform float uInvGamma;
uniform int   uFlatShading;

in  highp vec2  vUV0;
in  highp vec2  vUV1;
in  vec4  vColor;
flat in vec4 vColorFlat;
in  float vFogFactor;
out vec4  fragColor;

void main() {
    vec4 texColor = (uUseTexture != 0) ? texture(uTex0, vUV0) : vec4(1.0);
    vec4 base = (uFlatShading != 0) ? vColorFlat : vColor;
    vec4 c = texColor * base;
    if (c.a < uAlphaRef) discard;
    if (uUseLightmap != 0) c.rgb *= texture(uTex1, vUV1).rgb;
    if (uFogEnable != 0) c.rgb = mix(uFogColor.rgb, c.rgb, vFogFactor);
    c.rgb = pow(c.rgb, vec3(uInvGamma));
    fragColor = c;
}
)GLSL";