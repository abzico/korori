#version 330 core

uniform mat4 view_matrix;
uniform samplerCube cubemap_sampler;
uniform vec3 fog_color;
// color transition
// lower, then upper limit
uniform vec2 ctrans_limits;

in vec3 outin_texcoord;
out vec4 out_color;

void main()
{
  // check if limits leads to divide by zero, if so then disable transitioning
  float chk_denom = ctrans_limits.y - ctrans_limits.x;
  if (chk_denom > 0.0001f)
  {
    vec4 skybox_color = texture(cubemap_sampler, outin_texcoord);
    float factor = (outin_texcoord.y - ctrans_limits.x) / (ctrans_limits.y - ctrans_limits.x);
    factor = clamp(factor, 0.0f, 1.0f);
    out_color = mix(vec4(fog_color, 1.0f), skybox_color, factor);
  }
  else
  {
    out_color = texture(cubemap_sampler, outin_texcoord);
  }
}
