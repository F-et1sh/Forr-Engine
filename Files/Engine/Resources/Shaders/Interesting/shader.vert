#version 450 core

layout (location = 0) in vec3 a_Position;

layout (std430, binding = 0) readonly buffer SceneData {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[32];
} scene_data;

#ifdef FORR_USE_OPENGL
layout (location = 0) uniform int instance_index;
#else
layout(push_constant) uniform PushConstants {
	int instance_index;
} constants;
#endif

layout (location = 0) out vec3 v_LocalPos;
layout (location = 1) out vec3 v_WorldPos;

void main() {
	int idx =
#ifdef FORR_USE_OPENGL
		instance_index;
#else
		constants.instance_index;
#endif

	mat4 model_mat = scene_data.model_matrices[idx];
	
	float time = scene_data.projection_matrix[0][0] * 5.0f + scene_data.view_matrix[3][0];
	
	vec3 displaced_position = a_Position;
	displaced_position.y += sin(a_Position.x * 4.0f + time) * 0.1f;
	displaced_position.x += cos(a_Position.z * 4.0f + time) * 0.1f;

	vec4 world_pos = model_mat * vec4(displaced_position, 1.0f);
	gl_Position = scene_data.projection_matrix * scene_data.view_matrix * world_pos;
	
	v_LocalPos = displaced_position;
	v_WorldPos = world_pos.xyz;
}
