#version 330 core

// transformation matrices
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform vec3 light_position;
uniform float texcoord_repeat;

// vertex position attribute
in vec3 vertex_pos3d;

// texture coordinate attributes
in vec2 texcoord;
in vec3 normal;

out vec2 outin_texcoord;
out vec3 surface_normal;
out vec3 tocam_dir;
out vec3 tolight_dir;

void main()
{
  vec4 world_position = model_matrix * vec4(vertex_pos3d, 1.0);

  // process texcoord
  outin_texcoord = texcoord * texcoord_repeat;
  surface_normal = (model_matrix * vec4(normal, 0.0)).xyz;

  tolight_dir = light_position - world_position.xyz;

  // calculate direction to camera
  tocam_dir = (inverse(view_matrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz - world_position.xyz;

  // process vertex
  gl_Position = projection_matrix * view_matrix * world_position;
}
