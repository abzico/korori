#version 330 core

uniform vec4 texture_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform sampler2D texture_sampler;
uniform vec3 light_position;
uniform vec3 light_color;

// texture coordinate
in vec2 outin_texcoord;
in vec3 surface_normal;
in vec3 frag_pos;

// final color
out vec4 final_color;

void main()
{
  vec3 unit_normal = normalize(surface_normal);
  vec3 light_dir = normalize(light_position - frag_pos);

  float brightness = max(dot(unit_normal, light_dir), 0.0);
  vec3 diffuse = brightness * light_color;

  final_color = vec4(diffuse, 1.0) * texture(texture_sampler, outin_texcoord) * texture_color;
}
