#version 300 es

precision mediump float;

// texture unit
uniform sampler2D texture_sampler;

// texture coordinate
in vec2 outin_texcoord;

// final color
out vec4 final_color;

void main()
{
  final_color = texture(texture_sampler, outin_texcoord);
}
