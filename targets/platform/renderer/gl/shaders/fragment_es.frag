R"GLSL(
#version 100
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

varying highp vec2 vUV0; 
varying highp vec2 vUV1;
varying vec4 vColor;
varying float vFogFactor;

void main() {
    vec4 texColor = (uUseTexture != 0) ? texture2D(uTex0, vUV0) : vec4(1.0);
    vec4 c = texColor * vColor;
    
    if (c.a < uAlphaRef) discard;
    
    if (uUseLightmap != 0) c.rgb *= texture2D(uTex1, vUV1).rgb;
    if (uFogEnable != 0) c.rgb = mix(uFogColor.rgb, c.rgb, vFogFactor);
    c.rgb = pow(c.rgb, vec3(uInvGamma));
    //c.rgb = pow(max(c.rgb, 0.0), vec3(2.2));

    gl_FragColor = c;
}
)GLSL";
