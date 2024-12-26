# version 330

layout (location = 0) in vec3 Position;

uniform mat4 VP;

void main(){
    gl_Position = VP * vec4(Position, 1.0);
}