#version 300 es

//transformation matrices
uniform mat4 projection_matrix;
uniform mat4 model_matrix;

// vertex position attribute
in vec2 vertex_pos2d;

// texture coordinate attribute
in vec2 texcoord;
out vec2 outin_texcoord;

void main()
{
  // process texcoord
  outin_texcoord = texcoord;

  // process vertex
  gl_Position = projection_matrix * model_matrix * vec4(vertex_pos2d.x, vertex_pos2d.y, 0.0f, 1.0f);
}
