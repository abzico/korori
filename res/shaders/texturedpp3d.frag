#version 330 core

uniform mat4 view_matrix;
uniform sampler2D texture_sampler;
uniform vec3 light_position;
uniform vec3 light_color;
uniform float shine_damper;
uniform float reflectivity;

// texture coordinate
in vec2 outin_texcoord;
in vec3 surface_normal;
in vec3 frag_pos;
in vec3 tocam_dir;

// final color
out vec4 final_color;

void main()
{
  vec3 unit_normal = normalize(surface_normal);
  vec3 light_dir = normalize(light_position - frag_pos);

  // calculate light intensity
  float brightness = max(dot(unit_normal, light_dir), 0.0);
  vec3 diffuse = brightness * light_color;

  // calculate specular
  vec3 fromlight_dir = -light_dir;
  vec3 reflected_light_dir = reflect(fromlight_dir, unit_normal);
  float specular_factor = max(dot(reflected_light_dir, tocam_dir), 0.0);
  float damped_factor = pow(specular_factor, shine_damper);
  vec3 final_specular = damped_factor * light_color * reflectivity;

  final_color = vec4(diffuse, 1.0) * texture(texture_sampler, outin_texcoord) + vec4(final_specular, 1.0);
}
