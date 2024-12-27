#version 330

layout(location = 0) out vec4 FragColor;


in vec4 color;
in vec2 texCoords;

uniform sampler2D texSampler;

void main(){

    vec4 texColor = texture(texSampler, texCoords);

    FragColor = color * texColor;

}