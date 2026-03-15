#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in float aIndex;

layout (binding = 0) uniform UBO {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrix[2];
} ubo;

void main() {
	if (ubo.projection_matrix != mat4(0.0f))
		gl_Position = ubo.projection_matrix * ubo.view_matrix * ubo.model_matrix[int(aIndex)] * vec4(aPosition.xyz, 1.0f);
	else
		gl_Position = vec4(aPosition.xyz, 1.0f);
}
