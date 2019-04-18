#version 300 es

precision mediump float;

// texture color
uniform vec4 text_color;
uniform sampler2D texture_sampler;

// texture coordinate
in vec2 outin_texcoord;
// final color
out vec4 final_color;

void main()
{
  // get red component from texture (as we treat texture as 8-bit format for alpha)
  float red = texture(texture_sampler, outin_texcoord).r;

  // set alpha fragment
  final_color = vec4(1.0f, 1.0f, 1.0f, red) * text_color;
}
