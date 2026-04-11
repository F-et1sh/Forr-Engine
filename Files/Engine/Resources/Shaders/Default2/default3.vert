#version 450

layout (location = 0) in vec3 a_Position;

layout (binding = 0) uniform UBO {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[32];
} ubo;

uniform int model_index;

void main() {
	gl_Position = ubo.projection_matrix * ubo.view_matrix * ubo.model_matrices[model_index] * vec4(a_Position.xyz, 1.0f);
}
