#include "krr/graphics/spritesheet.h"
#include <stdlib.h>
#include <stdlib.h>
#include <stddef.h>
#include "krr/foundation/common.h"
#include "krr/graphics/texturedpp2d.h"
#include "krr/graphics/util.h"

static void init_defaults(KRR_SPRITESHEET* spritesheet);
static void free_internals(KRR_SPRITESHEET* spritesheet);

void init_defaults(KRR_SPRITESHEET* spritesheet)
{
  spritesheet->ltexture = NULL;
  spritesheet->clips = NULL;
  spritesheet->vertex_data_buffer = 0;
  spritesheet->index_buffers = NULL;
  spritesheet->vao = 0;
}

void free_internals(KRR_SPRITESHEET* spritesheet)
{
  if (spritesheet->ltexture != NULL)
  {
    // use KRR_TEXTURE's free function to help on freeing this
    KRR_TEXTURE_free(spritesheet->ltexture);
    spritesheet->ltexture = NULL;
  }

  if (spritesheet->clips != NULL)
  {
    // use vector's free function to help on freeing this
    vector_free(spritesheet->clips);
    spritesheet->clips = NULL;
  }

  // *vbo, ibo, and vao will be cleared in KRR_SPRITESHEET_free()
}

KRR_SPRITESHEET* KRR_SPRITESHEET_new(void)
{
  KRR_SPRITESHEET* out = malloc(sizeof(KRR_SPRITESHEET));
  init_defaults(out);

  // init
  out->ltexture = KRR_TEXTURE_new();
  out->clips = vector_new(1, sizeof(RECT));
  // no need to set free_element for vector this time as RECT is pure value type struct

  return out;
}

void KRR_SPRITESHEET_free(KRR_SPRITESHEET* spritesheet)
{
  // free sheet first
  KRR_SPRITESHEET_free_sheet(spritesheet);
  
  // free internals
  // the underlying managed KRR_TEXTURE and vector will be freed inside this call
  free_internals(spritesheet);

  free(spritesheet);
  spritesheet = NULL;
}

int KRR_SPRITESHEET_add_clipsprite(KRR_SPRITESHEET* spritesheet, const RECT* new_clip)
{
  // add a new clip then return its index
  vector_add(spritesheet->clips, (void*)new_clip);
  return spritesheet->clips->len - 1;
}

RECT KRR_SPRITESHEET_get_clip(KRR_SPRITESHEET* spritesheet, int index)
{
  return *(RECT*)vector_get(spritesheet->clips, index);
}

bool KRR_SPRITESHEET_generate_databuffer(KRR_SPRITESHEET* spritesheet)
{
  // if there is a texture loaded, and clips to make vertex data from
  if (spritesheet->ltexture->texture_id != 0 && spritesheet->clips->len > 0)
  {
    // allocate vertex buffer data
    const int total_sprites = spritesheet->clips->len;

    // yep we can use variable length array declaration in C99
    VERTEXTEX2D vertex_data[total_sprites * 4];
    // allocate buffer for index buffer
    spritesheet->index_buffers = malloc(total_sprites * 4 * sizeof(GLuint));

    // allocate vao
    glGenVertexArrays(1, &spritesheet->vao);
    // allocate vertex data buffer name
    glGenBuffers(1, &spritesheet->vertex_data_buffer);
    // allocate index buffer 
    glGenBuffers(total_sprites, spritesheet->index_buffers);

    // go through clips
    GLfloat texture_pwidth = spritesheet->ltexture->physical_width_;
    GLfloat texture_pheight = spritesheet->ltexture->physical_height_;
    GLuint sprite_indices[4] = {0, 0, 0, 0};

    for (int i=0; i<total_sprites; i++)
    {
      // initialize indices
      int base = i * 4;
      sprite_indices[0] = base;
      sprite_indices[1] = base + 1;
      sprite_indices[2] = base + 2;
      sprite_indices[3] = base + 3;

      // initialize vertex
      // get clip for current sprite
      RECT clip = *(RECT*)vector_get(spritesheet->clips, i);

      // calculate texture coordinate
      GLfloat tex_left = clip.x/texture_pwidth + 0.5/texture_pwidth;
      GLfloat tex_right = (clip.x+clip.w)/texture_pwidth - 0.5/texture_pwidth;
      GLfloat tex_top = clip.y/texture_pheight + 0.5/texture_pheight;
      GLfloat tex_bottom = (clip.y+clip.h)/texture_pheight - 0.5/texture_pheight;

      // top left
      vertex_data[sprite_indices[0]].position.x = 0.f;
      vertex_data[sprite_indices[0]].position.y = 0.f;

      vertex_data[sprite_indices[0]].texcoord.s = tex_left;
      vertex_data[sprite_indices[0]].texcoord.t = tex_top;

      // bottom left
      vertex_data[sprite_indices[1]].position.x = 0.f;
      vertex_data[sprite_indices[1]].position.y = clip.h;

      vertex_data[sprite_indices[1]].texcoord.s = tex_left;
      vertex_data[sprite_indices[1]].texcoord.t = tex_bottom;

      // bottom right
      vertex_data[sprite_indices[2]].position.x = clip.w;
      vertex_data[sprite_indices[2]].position.y = clip.h;

      vertex_data[sprite_indices[2]].texcoord.s = tex_right;
      vertex_data[sprite_indices[2]].texcoord.t = tex_bottom;

      // top right
      vertex_data[sprite_indices[3]].position.x = clip.w;
      vertex_data[sprite_indices[3]].position.y = 0.f;

      vertex_data[sprite_indices[3]].texcoord.s = tex_right;
      vertex_data[sprite_indices[3]].texcoord.t = tex_top;

      // bind sprite index buffer data
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spritesheet->index_buffers[i]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), sprite_indices, GL_STATIC_DRAW);

			GLenum error = glGetError();
			if (error != GL_NO_ERROR)
			{
				KRR_LOGE("Error opengl: %s", KRR_gputil_error_string(error));
				return false;
			}
    }

    // bind vertex data
    glBindBuffer(GL_ARRAY_BUFFER, spritesheet->vertex_data_buffer);
    glBufferData(GL_ARRAY_BUFFER, total_sprites * 4 * sizeof(VERTEXTEX2D), vertex_data, GL_STATIC_DRAW);

    // set up binding process for vao
    glBindVertexArray(spritesheet->vao);

      // enable all attribute pointers
      KRR_TEXSHADERPROG2D_enable_attrib_pointers(shared_textured_shaderprogram);

      // bind vertex data
      glBindBuffer(GL_ARRAY_BUFFER, spritesheet->vertex_data_buffer);

      // set texture coordinate attrib pointer
      KRR_TEXSHADERPROG2D_set_vertex_pointer(shared_textured_shaderprogram, sizeof(VERTEXTEX2D), (GLvoid*)offsetof(VERTEXTEX2D, texcoord));
      // set vertex data attrib pointer
      KRR_TEXSHADERPROG2D_set_texcoord_pointer(shared_textured_shaderprogram, sizeof(VERTEXTEX2D), (GLvoid*)offsetof(VERTEXTEX2D, position));

    // unbind vao
    glBindVertexArray(0);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			KRR_LOGE("Error opengl: %s", KRR_gputil_error_string(error));
			return false;
		}
  }
  else
  {
    if (spritesheet->ltexture->texture_id == 0)
    {
      KRR_LOGE("No texture to render with!");
    }

    if (spritesheet->clips->len <= 0)
    {
      KRR_LOGE("No clips to generate vertex data from");
    }

    return false;
  }

  return true;
}

void KRR_SPRITESHEET_free_sheet(KRR_SPRITESHEET* spritesheet)
{
  // delete vertex buffer
  if (spritesheet->vertex_data_buffer != 0)
  {
    glDeleteBuffers(1, &spritesheet->vertex_data_buffer);
    spritesheet->vertex_data_buffer = 0;
  }

  // delete index buffer
  if (spritesheet->index_buffers != NULL)
  {
    glDeleteBuffers(spritesheet->clips->len, spritesheet->index_buffers);
    // since we dynamically allocate for index buffers, we free them here
    free(spritesheet->index_buffers);
    spritesheet->index_buffers = NULL;
  }

  // delete vao
  if (spritesheet->vao != 0)
  {
    glDeleteVertexArrays(1, &spritesheet->vao);
    spritesheet->vao = 0;
  }

  // clear clips
  vector_clear(spritesheet->clips);
}

void KRR_SPRITESHET_bind_vao(KRR_SPRITESHEET* spritesheet)
{
  // bind vao
  glBindVertexArray(spritesheet->vao);

  // bind texture
  glBindTexture(GL_TEXTURE_2D, spritesheet->ltexture->texture_id);
}

void KRR_SPRITESHEET_render_sprite(KRR_SPRITESHEET* spritesheet, int index, GLfloat x, GLfloat y)
{
	// move to rendering position
  glm_translate(shared_textured_shaderprogram->model_matrix, (vec3){x, y, 0.f});
  // issue update to gpu
  KRR_TEXSHADERPROG2D_update_view_matrix(shared_textured_shaderprogram);

  // bind index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spritesheet->index_buffers[index]);
  // draw using data from vertex and index buffer
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);
}

void KRR_SPRITESHEET_unbind_vao(KRR_SPRITESHEET* spritesheet)
{
  glBindVertexArray(0);
}
