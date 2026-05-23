#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;

layout(location = 0) out vec4 i_Position;
layout(location = 1) out vec3 i_Normal;

layout (std430, set = 0, binding = 0) readonly buffer SceneData {
mat4 projection_matrix;
mat4 view_matrix;
mat4 model_matrices[];
} scene_data;

#ifdef FORR_USE_OPENGL
layout (location = 0) uniform int instance_index;
#else
layout(push_constant) uniform PushConstants {
	int instance_index;
} constants;
#endif

void main() {
#ifdef FORR_USE_OPENGL
	gl_Position = scene_data.projection_matrix * scene_data.view_matrix * scene_data.model_matrices[instance_index] * vec4(a_Position.xyz, 1.0f);
#else
	i_Position = scene_data.model_matrices[constants.instance_index] * vec4(a_Position.xyz, 1.0f);
	gl_Position = scene_data.projection_matrix * scene_data.view_matrix * i_Position;
	i_Normal = a_Normal;
#endif
}