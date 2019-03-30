#version 330 core

uniform mat4 view_matrix;
uniform sampler2D texture_sampler;
uniform int light_num;
uniform float light_attenuation[4];
uniform vec3 light_color[4];
uniform float shine_damper;
uniform float reflectivity;
uniform vec3 ambient_color;
uniform vec3 sky_color;
uniform float fog_enabled;

// texture coordinate
in vec2 outin_texcoord;
in vec3 surface_normal;
in vec3 tocam_dir;
in vec3 tolight_dir[4];
in float visibility;

// final color
out vec4 final_color;

void main()
{
  vec3 unit_normal = normalize(surface_normal);

  vec3 total_diffuse = vec3(0.0f);
  vec3 total_specular = vec3(0.0f);

  for (int i=0; i<light_num; ++i)
  {
    // use equation attenuation_factor = 1 + c*(d^2)
    float dist_sq = pow(tolight_dir[i].x, 2) + pow(tolight_dir[i].y, 2) + pow(tolight_dir[i].z, 2);
    float attenuation_denom = 1.0f + light_attenuation[i]*dist_sq;
    
    vec3 light_dir = normalize(tolight_dir[i]);
    float brightness = max(dot(unit_normal, light_dir), 0.2f);
    total_diffuse = total_diffuse + (brightness * light_color[i] + ambient_color) / attenuation_denom;

    // calculate specular
    // note: normalize tocam_dir here as there's no guaruntee it will be unit vector after interpolation resulting from vertex shader
    vec3 fromlight_dir = -light_dir;
    vec3 reflected_light_dir = reflect(fromlight_dir, unit_normal);
    float specular_factor = max(dot(reflected_light_dir, normalize(tocam_dir)), 0.0f);
    float damped_factor = pow(specular_factor, shine_damper);
    total_specular = total_specular + (damped_factor * light_color[i] * reflectivity) / attenuation_denom;
  }

  final_color = vec4(total_diffuse, 1.0f) * texture(texture_sampler, outin_texcoord) + vec4(total_specular, 1.0f);
  if (fog_enabled == 1.0f)
  {
    final_color = mix(vec4(sky_color,1.0f), final_color, visibility);
  }
}
