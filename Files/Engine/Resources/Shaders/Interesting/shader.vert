#version 450 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;

layout (location = 0) out vec3 i_Normal;
layout (location = 1) out vec3 v_LocalPos;
layout (location = 2) out vec3 v_WorldPos;

#define FORR_BINDING_COUNT_PER_SET 4

#ifdef FORR_USE_OPENGL
    #define FORR_LAYOUT(set_index, binding_index) layout(binding = (set_index * FORR_BINDING_COUNT_PER_SET) + binding_index)
#else
    #define FORR_LAYOUT(set_index, binding_index) layout(set = set_index, binding = binding_index)
#endif

struct GPULight {
	//uint32_t type;
	
	//float range;
	//float inner_cone;
	//float outer_cone;
	
	vec4 position;
	vec4 direction;
	vec4 color_intensity;
};

FORR_LAYOUT(0, 0) readonly buffer SceneData {
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 model_matrices[];
} scene_data;

FORR_LAYOUT(0, 1) readonly buffer LightData {
uint lights_count;
GPULight lights[];
} light_data;

FORR_LAYOUT(1, 0) readonly buffer MaterialData {
mat4 some_data[];
} material_data;

#ifdef FORR_USE_OPENGL
layout (location = 0) uniform int instance_index;
#else
layout(push_constant) uniform PushConstants {
	int instance_index;
} constants;
#endif

void main() {
	int index =
#ifdef FORR_USE_OPENGL
		instance_index;
#else
		constants.instance_index;
#endif

	mat4 model_matrix = scene_data.model_matrices[index];
	
	//float time = scene_data.projection_matrix[0][0] * 5.0f + scene_data.view_matrix[3][0];
	
	//vec3 displaced_position = a_Position;
	//displaced_position.y += sin(a_Position.x * 4.0f + time) * 0.1f;
	//displaced_position.x += cos(a_Position.z * 4.0f + time) * 0.1f;

	vec4 world_pos = model_matrix * vec4(a_Position, 1.0f);
	gl_Position = scene_data.projection_matrix * scene_data.view_matrix * world_pos;
	
	v_LocalPos = a_Position;
	v_WorldPos = world_pos.xyz;

	i_Normal = a_Normal;
}
