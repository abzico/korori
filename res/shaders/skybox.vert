#version 330 core

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

in vec3 vertex_pos3d;
out vec3 outin_texcoord;

void main()
{
  outin_texcoord = vertex_pos3d;

  vec4 pos = projection_matrix * view_matrix * vec4(vertex_pos3d, 1.0f);
  // trick to always have skybox rendered at the deepest of depth buffer (i.e. 1.0f)
  gl_Position = pos.xyww;
}
