#version 330

in vec3 worldPos;

vec2 satv(vec2 v){
    return clamp(v, 0, 1);
}

float max2(vec2 v){
    return max(v[0], v[1]);
}

layout(location = 0) out vec4 FragColor;

uniform float gGridCCellSize = 0.005;
uniform vec4 gGridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec4 gGridColorThick = vec4(0.0, 0.0, 0.0, 1.0);

void main(){

    vec2 dvx = vec2(dFdx(worldPos.x), dFdy(worldPos.x));
    vec2 dvy = vec2(dFdx(worldPos.z), dFdy(worldPos.z));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);

    dudv *= 4.0;

    vec2 mod_div_dudv = mod(worldPos.xz, gGridCCellSize) / dudv;
    float Lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));

    vec4 color;

    color = gGridColorThick;
    color.a *= Lod0a;

    FragColor = color;
}