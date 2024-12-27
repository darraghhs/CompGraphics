# version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 UV;

uniform mat4 VP;
uniform float minHeight;
uniform float maxHeight;
out vec4 color;
out vec2 texCoords;

void main(){
    gl_Position = VP * vec4(Position, 1.0);

    float heightRatio = (Position.y - minHeight) / (maxHeight - minHeight);

    float c = heightRatio * 0.8 + 0.2;

    color = vec4(c, c, c, 1.0);

    texCoords = UV;

}