# version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec3 inNormal;

uniform mat4 MVP;
uniform float minHeight;
uniform float maxHeight;

out vec4 color;
out vec2 texCoords;
out vec3 worldPos;
out vec3 normal;


void main(){
    gl_Position = MVP * vec4(Position, 1.0);

    float heightRatio = (Position.y - minHeight) / (maxHeight - minHeight);

    float c = heightRatio * 0.8 + 0.2;

    color = vec4(c, c, c, 1.0);

    texCoords = UV;

    worldPos = Position;

    normal = inNormal;

}