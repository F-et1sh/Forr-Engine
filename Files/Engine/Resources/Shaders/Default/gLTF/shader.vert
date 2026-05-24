#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;

layout(location = 0) out vec3 i_Position;
layout(location = 1) out vec3 i_Normal;

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

	vec4 position = scene_data.model_matrices[index] * vec4(a_Position.xyz, 1.0f);
	gl_Position = scene_data.projection_matrix * scene_data.view_matrix * position;
	i_Position = position.xyz;
	i_Normal = a_Normal;
}