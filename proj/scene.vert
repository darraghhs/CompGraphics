#version 330

out vec3 worldPos;

uniform mat4 VP;
uniform vec3 camPos;

const vec3 pos[4] = vec3[4](
    vec3(-1.0, 0.0, -1.0),  // bottom left
    vec3(1.0, 0.0, -1.0),   // bottom right
    vec3(1.0, 0.0, 1.0),   // top right
    vec3(-1.0, 0.0, 1.0)    // top left
);

const int indices[6] = int[6](0, 1, 2, 2, 3, 0);

void main(){

    int index = indices[gl_VertexID];

    vec3 vPos3 = pos[index];

    vPos3.x += camPos.x;
    vPos3.z += camPos.z;

    vec4 vPos4 = vec4(vPos3, 1.0);

    gl_Position = VP * vPos4;

    worldPos = vPos3;
    
}