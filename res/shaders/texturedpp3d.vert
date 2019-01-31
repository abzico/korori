#version 150

// transformation matrices
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

// vertex position attribute
in vec3 vertex_pos3d;

// texture coordinate attributes
in vec2 texcoord;
out vec2 outin_texcoord;

void main()
{
  // process texcoord
  outin_texcoord = texcoord;

  // process vertex
  gl_Position = projection_matrix * view_matrix * model_matrix * vec4(vertex_pos3d.x, vertex_pos3d.y, vertex_pos3d.z, 1.0);
}
