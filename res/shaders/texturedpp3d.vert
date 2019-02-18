#version 330 core

// transformation matrices
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

// vertex position attribute
in vec3 vertex_pos3d;

// texture coordinate attributes
in vec2 texcoord;
in vec3 normal;

out vec2 outin_texcoord;
out vec3 surface_normal;
out vec3 frag_pos;

void main()
{
  vec4 world_position = model_matrix * vec4(vertex_pos3d, 1.0);

  // process texcoord
  outin_texcoord = texcoord;
  surface_normal = (model_matrix * vec4(normal, 0.0)).xyz;

  frag_pos = world_position.xyz;

  // process vertex
  gl_Position = projection_matrix * view_matrix * world_position;
}
