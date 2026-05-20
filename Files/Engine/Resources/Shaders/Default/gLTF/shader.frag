#version 450 core
layout(location = 0) out vec4 fragColor;

layout (std430, binding = 0) readonly buffer SceneData {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[];
} scene_data;

void main() {
	fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}