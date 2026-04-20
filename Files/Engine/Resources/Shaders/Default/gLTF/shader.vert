#version 450 core

layout (location = 0) in vec3 a_Position;

layout (std430, binding = 0) readonly buffer SceneData {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[32];
} scene_data;

#ifdef FORR_USE_OPENGL
layout (location = 0) uniform int model_index;
#else
layout(push_constant) uniform PushConstants {
	int index;
} constants;
#endif

void main() {
#ifdef FORR_USE_OPENGL
	gl_Position = scene_data.projection_matrix * scene_data.view_matrix * scene_data.model_matrices[model_index] * vec4(a_Position.xyz, 1.0f);
#else
	gl_Position = scene_data.projection_matrix * scene_data.view_matrix * scene_data.model_matrices[constants.index] * vec4(a_Position.xyz, 1.0f);
#endif
}