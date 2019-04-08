#version 300 es

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform lowp int light_num;
uniform vec3 light_position[4];
uniform lowp float fog_enabled;
uniform float fog_density;
uniform float fog_gradient;
// packed x-axis in first two vecs, and y-axis for second two vecs
// format is (min_u, max_u), (min_v, max_v)
uniform vec4 packed_clip_texture_uv;

in vec3 vertex_pos3d;
in vec2 texcoord;
in vec3 normal;

out vec2 outin_texcoord;
out vec3 surface_normal;
out vec3 tocam_dir;
out vec3 tolight_dir[4];
out float visibility;

void main()
{
  vec4 world_position = model_matrix * vec4(vertex_pos3d, 1.0f);

  // process texcoord
	if (packed_clip_texture_uv.x == 0.0 &&
			packed_clip_texture_uv.y == 0.0 &&
			packed_clip_texture_uv.z == 0.0 &&
			packed_clip_texture_uv.w == 0.0)
	{
		outin_texcoord = texcoord;
	}
	// this is sprite inside the sheet
	else
	{
		// calculate result texcoord xy
		outin_texcoord.x = (packed_clip_texture_uv.y - packed_clip_texture_uv.x) * texcoord.x + packed_clip_texture_uv.x;
		outin_texcoord.y = (packed_clip_texture_uv.w - packed_clip_texture_uv.z) * texcoord.y + packed_clip_texture_uv.z;
	}

  surface_normal = (model_matrix * vec4(normal, 0.0f)).xyz;

  for (int i=0; i<light_num; ++i)
  {
    tolight_dir[i] = light_position[i] - world_position.xyz;
  }

  // calculate direction to camera
  tocam_dir = (inverse(view_matrix) * vec4(0.0f, 0.0f, 0.0f, 1.0f)).xyz - world_position.xyz;

  // calculate fog
  // from eqaution e^(-((distance*density)^gradient)) 
  if (fog_enabled == 1.0f)
  {
    vec4 position_rel_to_cam = view_matrix * world_position;
    float dst = length(position_rel_to_cam.xyz);
    visibility = exp(-pow(dst*fog_density, fog_gradient));
  }

  // process vertex
  gl_Position = projection_matrix * view_matrix * world_position;
}
