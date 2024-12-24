#version 330

uniform mat4 VP;

const vec3 Pos[4] = vec3[4](
    vec3(-1.0, 0.0, -1.0),  // bottom left
    vec3(1.0, 0.0, -1.0),   // bottom right
    vec3(-1.0, 0.0, 1.0),   // top right
    vec3(-1.0, 0.0, 1.0),   // top left

);

const int indices[6] = int[6](0, 2, 1, 2, 0, 3);

void main(){

    int Index = Indices[gl_VertexID];
    vec4 vpos = vec4(Pos[Index], 1.0);
    gl_Position = VP * vPos
    
}