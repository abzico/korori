#version 330 core

uniform mat4 view_matrix;

uniform vec3 light_color;
uniform float shine_damper;
uniform float reflectivity;
uniform vec3 ambient_color;
uniform vec3 sky_color;
uniform float fog_enabled;
uniform float multitexture_enabled;

// this texture will be used as background texture
// in multitexturing process if enabled via multitexture_enabled
uniform sampler2D texture_sampler;
uniform sampler2D multitexture_texture_r;
uniform sampler2D multitexture_texture_g;
uniform sampler2D multitexture_texture_b;
uniform sampler2D multitexture_blendmap;

// texture coordinate
in vec2 outin_texcoord;
in vec2 tiled_texcoord;
in vec3 surface_normal;
in vec3 tocam_dir;
in vec3 tolight_dir;
in float visibility;

// final color
out vec4 final_color;

void main()
{
  vec3 unit_normal = normalize(surface_normal);
  vec3 light_dir = normalize(tolight_dir);

  float brightness = max(dot(unit_normal, light_dir), 0.0f);
  vec3 diffuse = brightness * light_color + ambient_color;
  diffuse = clamp(diffuse, 0.0f, 1.0f);

  vec4 terrain_color = texture(texture_sampler, tiled_texcoord);

  // calculate multitexturing
  if (multitexture_enabled == 1.0f)
  {
    vec4 blendmap_color = texture(multitexture_blendmap, outin_texcoord);
    float multi_background_amount = 1.0f - (blendmap_color.r + blendmap_color.g + blendmap_color.b);
    vec4 multi_background_color = terrain_color * multi_background_amount;
    vec4 multi_texture_r_color = texture(multitexture_texture_r, tiled_texcoord) * blendmap_color.r;
    vec4 multi_texture_g_color = texture(multitexture_texture_g, tiled_texcoord) * blendmap_color.g;
    vec4 multi_texture_b_color = texture(multitexture_texture_b, tiled_texcoord) * blendmap_color.b;
    terrain_color = multi_background_color + multi_texture_r_color + multi_texture_g_color + multi_texture_b_color;
  }

  // calculate specular
  // note: normalize tocam_dir here as there's no guaruntee it will be unit vector after interpolation resulting from vertex shader
  vec3 fromlight_dir = -light_dir;
  vec3 reflected_light_dir = reflect(fromlight_dir, unit_normal);
  float specular_factor = max(dot(reflected_light_dir, normalize(tocam_dir)), 0.0f);
  float damped_factor = pow(specular_factor, shine_damper);
  vec3 final_specular = damped_factor * light_color * reflectivity;

  final_color = vec4(diffuse, 1.0f) * terrain_color + vec4(final_specular, 1.0f);
  if (fog_enabled == 1.0f)
  {
    final_color = mix(vec4(sky_color,1.0f), final_color, visibility);
  }
}
