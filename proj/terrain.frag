#version 330

layout(location = 0) out vec4 FragColor;


in vec4 color;
in vec2 texCoords;
in vec3 normal;

uniform sampler2D texSampler;

uniform vec3 reversedLightDir;

void main(){

    vec4 texColor = texture(texSampler, texCoords);

    vec3 normal_ = normalize(normal);

    float diffuse = dot(normal_, reversedLightDir);

    diffuse = max(0.3f, diffuse);

    FragColor = color * texColor * (diffuse * 2.0f);

}