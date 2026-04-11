#version 460
layout(location = 0) out vec4 fragColor;

layout (std140, binding = 1) uniform Material {
	vec3 color;
}material;

void main() {
	fragColor = vec4(material.color, 1.0f);
}