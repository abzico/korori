#version 330 core

uniform mat4 view_matrix;
uniform samplerCube cubemap_sampler;

in vec3 outin_texcoord;
out vec4 out_color;

void main()
{
  out_color = texture(cubemap_sampler, outin_texcoord);
}
