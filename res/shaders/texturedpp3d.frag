#version 330 core

uniform mat4 view_matrix;
uniform sampler2D texture_sampler;
uniform vec3 light_color;
uniform float shine_damper;
uniform float reflectivity;
uniform vec3 ambient_color;

// texture coordinate
in vec2 outin_texcoord;
in vec3 surface_normal;
in vec3 tocam_dir;
in vec3 tolight_dir;

// final color
out vec4 final_color;

void main()
{
  vec4 texcolor = texture(texture_sampler, outin_texcoord);
  if (texcolor.a <= 0.0)
  {
    discard;
  }
  else
  {
    vec3 unit_normal = normalize(surface_normal);
    vec3 light_dir = normalize(tolight_dir);

    float brightness = max(dot(unit_normal, light_dir), 0.0);
    vec3 diffuse = brightness * light_color + ambient_color;
    diffuse = clamp(diffuse, 0.0f, 1.0f);

    // calculate specular
    // note: normalize tocam_dir here as there's no guaruntee it will be unit vector after interpolation resulting from vertex shader
    vec3 fromlight_dir = -light_dir;
    vec3 reflected_light_dir = reflect(fromlight_dir, unit_normal);
    float specular_factor = max(dot(reflected_light_dir, normalize(tocam_dir)), 0.0);
    float damped_factor = pow(specular_factor, shine_damper);
    vec3 final_specular = damped_factor * light_color * reflectivity;

    final_color = vec4(diffuse, 1.0) * texcolor + vec4(final_specular, 1.0);
  }
}
