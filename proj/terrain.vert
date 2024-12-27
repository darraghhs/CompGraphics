# version 330

layout (location = 0) in vec3 Position;

uniform mat4 VP;
uniform float maxHeight;
out vec4 color;

void main(){
    gl_Position = VP * vec4(Position, 1.0);

    color = vec4(Position.y / maxHeight);
}