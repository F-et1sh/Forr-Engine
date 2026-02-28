#version 450

layout (location = 0) in vec3 aPosition;

layout (binding = 0) uniform UBO {
	mat4 projection_matrix;
	mat4 model_matrix;
	mat4 view_matrix;
} ubo;

void main() {
	gl_Position = ubo.projection_matrix * ubo.view_matrix * ubo.model_matrix * vec4(aPosition.xyz, 1.0f);
}
