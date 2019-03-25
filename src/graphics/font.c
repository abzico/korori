#include "krr/graphics/font.h"
#include <stdlib.h>
#include <stddef.h>
#include "krr/foundation/log.h"
#include <vector/vector.h>
#include "krr/graphics/texture_internals.h"
#include FT_BITMAP_H
#include "krr/graphics/shaderprog.h"
#include "krr/graphics/font_internals.h"
#include "krr/graphics/fontpp2d.h"

// spacing when render between character in pixel
#define BETWEEN_CHAR_SPACING 4

struct KRR_FONTSHADERPROG2D_* shared_font_shaderprogram = NULL;

// freetype font library
// single shared variable for all instance of KRR_FONT during lifetime of application
static FT_Library freetype_library_ = NULL;

static void init_defaults_(KRR_FONT* font);
static void free_internals_(KRR_FONT* font);
static void report_freetype_error_(const FT_Error* error);

void init_defaults_(KRR_FONT* font)
{
  font->spritesheet = NULL;
  font->space = 0.f;
  font->line_height = 0.f;
  font->newline = 0.f;
}

void report_freetype_error_(const FT_Error* error)
{
  KRR_LOGE("FreeType error with code: %X", *error);
}

void free_internals_(KRR_FONT* font)
{
  KRR_SPRITESHEET_free(font->spritesheet);

  font->space = 0.f;
  font->line_height = 0.f;
  font->newline = 0.f;
}

KRR_FONT* KRR_FONT_new(void)
{
  KRR_FONT *out = malloc(sizeof(KRR_FONT));
  init_defaults_(out);

  // set spritesheet
  out->spritesheet = KRR_SPRITESHEET_new();

  return out;
}

void KRR_FONT_free(KRR_FONT* font)
{
  free_internals_(font);

  free(font);
  font = NULL;
}

bool KRR_FONT_load_bitmap(KRR_FONT* font, const char* path)
{
  // expect image that is grayscale, in 16x16 ASCII order

  // now KRR_TEXTURE supports 8-bit grayscale image
  // so black color is 0x0
  const GLubyte black_pixel = 0x00;

  // get rid of the font if it exists
  KRR_FONT_free_font(font);

  // image pixels loaded
  KRR_TEXTURE* texture = font->spritesheet->ltexture;
  // load from grayscale 8-bit image
  if (!KRR_TEXTURE_load_pixels_from_file8(texture, path))
  {
    KRR_LOGE("Unable to load pixels from file");
    return false;
  }

  // get cell dimensions
  // image has 16x16 cells
  GLfloat cell_width = texture->width / 16.f;
  GLfloat cell_height = texture->height / 16.f;

  // get letter top and bottom
  GLuint top = cell_height;
  GLuint bottom = 0.f;
  GLuint a_bottom = 0.f;

  // current pixel coordinate
  int p_x = 0;
  int p_y = 0;

  // base cell offsets
  int b_x = 0;
  int b_y = 0;

  // begin parsing bitmap font
  GLuint current_char = 0;
  RECT next_clip = { 0.f, 0.f, cell_width, cell_height };

  // go through cell rows
  for (int row = 0; row < 16; row++)
  {
    // go through each cell column in the row
    for (int col = 0; col < 16; col++)
    {
      // begin cell parsing
      // set base offsets
      b_x = cell_width * col;
      b_y = cell_height * row;
      
      // initialize clip
      next_clip.x = b_x;
      next_clip.y = b_y;

      next_clip.w = cell_width;
      next_clip.h = cell_height;

      // find boundary for character
      // left side
      for (int p_col = 0; p_col < cell_width; p_col++)
      {
        for (int p_row = 0; p_row < cell_height; p_row++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non-background pixel found
          if (KRR_TEXTURE_get_pixel8(texture, p_x, p_y) != black_pixel)
          {
            // set sprite's x offset
            next_clip.x = p_x;

            // break the loop
            p_col = cell_width;
            p_row = cell_height;
          }
        }
      }

      // right side
      for (int p_col = cell_width-1; p_col >= 0; p_col--)
      {
        for (int p_row = 0; p_row < cell_height; p_row++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non background pixel found
          if (KRR_TEXTURE_get_pixel8(texture, p_x, p_y) != black_pixel)
          {
            // set sprite's width
            next_clip.w = (p_x - next_clip.x) + 1;

            // break the loop
            p_col = -1;
            p_row = cell_height;
          }
        }
      }

      // find top
      for (int p_row = 0; p_row < cell_height; p_row++)
      {
        for (int p_col = 0; p_col < cell_width; p_col++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non background pixel found
          if (KRR_TEXTURE_get_pixel8(texture, p_x, p_y) != black_pixel)
          {
            // new top found
            if (p_row < top)
            {
              top = p_row;
            }

            // break the loop
            p_row = cell_height;
            p_col = cell_width;
          }
        }
      }

      // find bottom
      for (int p_row = cell_height - 1; p_row >= 0; p_row--)
      {
        for (int p_col = 0; p_col < cell_width; p_col++)
        {
          // set pixel offset
          p_x = b_x + p_col;
          p_y = b_y + p_row;

          // non background pixel found
          if (KRR_TEXTURE_get_pixel8(texture, p_x, p_y) != black_pixel)
          {
            // set baseline
            if (current_char == 'A')
            {
              a_bottom = p_row;
            }

            // new bottom found
            if (p_row > bottom)
            {
              bottom = p_row;
            }

            // break the loop
            p_row = -1;
            p_col = cell_width;
          }
        }
      }

      // go to the next character
      vector_add(font->spritesheet->clips, &next_clip);
      current_char++;
    }
  }

  // set top
  // by lopping off extra height from all the character sprites
  vector* clips = font->spritesheet->clips;
  for (int t = 0; t < clips->len; t++)
  {
    // get RECT
    RECT* rect = (RECT*)vector_get(clips, t);

    // update back
    rect->y += top;
    rect->h -= top;
  }

  // create texture from manipulated pixels
  if (!KRR_TEXTURE_load_texture_from_precreated_pixels8(texture))
  {
    KRR_LOGE("Unable to create texture from precreated pixels");
    return false;
  }

  // build vertex buffer from spritesheet data
  if (!KRR_SPRITESHEET_generate_databuffer(font->spritesheet))
  {
    KRR_LOGE("Unable to generate databuffer for spritesheet");
    return false;
  }

  // set texture wrap
  glBindTexture(GL_TEXTURE_2D, texture->texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glBindTexture(GL_TEXTURE_2D, 0);

  // setup vao's binding
  //  use vao of spritesheet as KRR_FONT relies on it
  KRR_SPRITESHEET* ss = font->spritesheet;
  glBindVertexArray(ss->vao);

    // enable all attribute pointers
    KRR_FONTSHADERPROG2D_enable_attrib_pointers(shared_font_shaderprogram);

    // bind vertex data
    glBindBuffer(GL_ARRAY_BUFFER, ss->vertex_data_buffer);

    // set texture coordinate attrib pointer
    KRR_FONTSHADERPROG2D_set_texcoord_pointer(shared_font_shaderprogram, sizeof(VERTEXTEX2D), (GLvoid*)offsetof(VERTEXTEX2D, texcoord));
    // set vertex data attrib pointer
    KRR_FONTSHADERPROG2D_set_vertex_pointer(shared_font_shaderprogram, sizeof(VERTEXTEX2D), (GLvoid*)offsetof(VERTEXTEX2D, position));

    // note: binding ibo will be done at rendering time depends on which character to render at that time

  glBindVertexArray(0);

  // set spacing variables
  font->space = cell_width / 2.f;
  font->newline = a_bottom - top;
  font->line_height = bottom - top;

  return true;
}

bool KRR_FONT_load_freetype(KRR_FONT* font, const char* path, GLuint pixel_size)
{
  // free previously loaded font
  KRR_FONT_free_font(font);

  // init freetype
  FT_Error error = 0;

  error = FT_Init_FreeType(&freetype_library_);
  if (error)
  {
    report_freetype_error_(&error);
    return false;
  }

  // get cell dimensions
  GLuint cell_width = 0;
  GLuint cell_height = 0;
  int max_bearing = 0;
  int min_hang = 0;

  // character data
  // this is an array of pointer to KRR_TEXTURE
  KRR_TEXTURE* bitmaps[256];
  FT_Glyph_Metrics metrics[256];
  
  // load face
  FT_Face face = NULL;
  error = FT_New_Face(freetype_library_, path, 0, &face);
  // error 0 means success for FreeType
  if (error)
  {
    report_freetype_error_(&error);
    return false;
  }

  // set face size
  error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
  if (error)
  {
    report_freetype_error_(&error);
    return false;
  }

  // go through extended ASCII to get glyph data
  for (int i=0; i<256; i++)
  {
    // load and render glyph
    error = FT_Load_Char(face, i, FT_LOAD_RENDER);
    if (error)
    {
      report_freetype_error_(&error);
      // report error but still keep going until finish all glyphs
      continue;
    }

    // get metrics
    metrics[i] = face->glyph->metrics;

    // initialize KRR_TEXTURE inside bitmaps array
    bitmaps[i] = KRR_TEXTURE_new();

    // copy glyph bitmap
    KRR_TEXTURE_copy_pixels8(bitmaps[i], face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows);

    // calculate max bearing
    // as in http://lazyfoo.net/tutorials/OpenGL/23_freetype_fonts/index.php
    // author claims that 1 point = 64 pixels
    if (metrics[i].horiBearingY / 64 > max_bearing)
    {
      max_bearing = metrics[i].horiBearingY / 64;
    }

    // calculate max width
    if (metrics[i].width / 64 > cell_width)
    {
      cell_width = metrics[i].width / 64;
    }

    // calculate glyph hang
    int glyph_hang = (metrics[i].horiBearingY - metrics[i].height) / 64;
    if (glyph_hang < min_hang)
    {
      min_hang = glyph_hang;
    }
  }

  // create bitmap font
  cell_height = max_bearing - min_hang;
  // 16 by 16 cells in creation
  KRR_TEXTURE_create_pixels8(font->spritesheet->ltexture, cell_width * 16, cell_height * 16);
  
  // begin creating bitmap font
  GLuint current_char = 0;
  RECT next_clip = { 0.f, 0.f, cell_width, cell_height };

  // blitting coordinates
  int b_x = 0;
  int b_y = 0;

  // go through cell rows
  for (unsigned int rows = 0; rows < 16; rows++)
  {
    // go through each cell column in the row
    for (unsigned int cols = 0; cols < 16; cols++)
    {
      // set base offsets
      b_x = cell_width * cols;
      b_y = cell_height * rows;

      // initialize clip
      next_clip.x = b_x;
      next_clip.y = b_y;
      next_clip.w = metrics[current_char].width / 64;
      next_clip.h = cell_height;

      // blit character
      KRR_TEXTURE_blit_pixels8(bitmaps[current_char], b_x, b_y + max_bearing - metrics[current_char].horiBearingY / 64, font->spritesheet->ltexture);

      // go to the next character
      vector_add(font->spritesheet->clips, &next_clip);
      current_char++;
    }
  }

  // we are done with bitmaps
  // free them all now
  // free all allocated bitmaps
  for (int i=0; i<256; i++)
  {
    if (bitmaps[i] != NULL)
    {
      KRR_TEXTURE_free(bitmaps[i]);
      bitmaps[i] = NULL;
    }
  }

  // make texture power of two
  KRR_TEXTURE_pad_pixels8(font->spritesheet->ltexture);

  // create texture
  if (!KRR_TEXTURE_load_texture_from_precreated_pixels8(font->spritesheet->ltexture))
  {
    KRR_LOGE("Unable to create texture from pre-created pixels8");
    return false;
  }

  // build vertex buffer from sprite sheet data
  if (!KRR_SPRITESHEET_generate_databuffer(font->spritesheet))
  {
    KRR_LOGE("Unable to geneate databuffer");
    return false;
  }

  // set texture wrap
  glBindTexture(GL_TEXTURE_2D, font->spritesheet->ltexture->texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glBindTexture(GL_TEXTURE_2D, 0);

  // setup vao's binding
  //  use vao of spritesheet as KRR_FONT relies on it
  KRR_SPRITESHEET* ss = font->spritesheet;
  glBindVertexArray(ss->vao);

    // enable all attribute pointers
    KRR_FONTSHADERPROG2D_enable_attrib_pointers(shared_font_shaderprogram);

    // bind vertex data
    glBindBuffer(GL_ARRAY_BUFFER, ss->vertex_data_buffer);

    // set texture coordinate attrib pointer
    KRR_FONTSHADERPROG2D_set_texcoord_pointer(shared_font_shaderprogram, sizeof(VERTEXTEX2D), (GLvoid*)offsetof(VERTEXTEX2D, texcoord));
    // set vertex data attrib pointer
    KRR_FONTSHADERPROG2D_set_vertex_pointer(shared_font_shaderprogram, sizeof(VERTEXTEX2D), (GLvoid*)offsetof(VERTEXTEX2D, position));

    // note: binding ibo will be done at rendering time depends on which character to render at that time

  glBindVertexArray(0);

  // set spacing variables
  font->space = cell_width / 2.0f;
  font->line_height = cell_height;
  font->newline = max_bearing;

  // free face
  FT_Done_Face(face);
  face = NULL;

  // free freetype
  FT_Done_FreeType(freetype_library_);
  freetype_library_ = NULL;

  return true;
}

void KRR_FONT_free_font(KRR_FONT* font)
{
  // clear the sheet
  KRR_SPRITESHEET_free_sheet(font->spritesheet);
  // clear the underlying 
  KRR_TEXTURE_free_internal_texture(font->spritesheet->ltexture);

  // reinitialize spacing constants
  font->space = 0.f;
  font->line_height = 0.f;
  font->newline = 0.f;
}

void KRR_FONT_bind_vao(KRR_FONT* font)
{
  KRR_SPRITESHEET* ss = font->spritesheet;

  // as KRR_FONT relies on KRR_SPRITESHEET, thus we bind KRR_SPRITESHEET's vao
  glBindVertexArray(ss->vao);

  // bind texture
  glBindTexture(GL_TEXTURE_2D, ss->ltexture->texture_id);
}

void KRR_FONT_render_text(KRR_FONT* font, const char* text, GLfloat x, GLfloat y)
{
  // if there is texture to render from
  if (font->spritesheet->ltexture->texture_id != 0)
  {
    // get spritesheet
    KRR_SPRITESHEET* ss = font->spritesheet;
    // get texture id
    GLuint texture_id = ss->ltexture->texture_id;

    // draw position
    GLfloat render_x = x;
    GLfloat render_y = y;

    // translate to rendering position
    glm_translate(shared_font_shaderprogram->model_matrix, (vec3){x, y, 0.f});
    // issue update to gpu
    KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);

    // set texture
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // go through string
    int text_length = strlen(text);
    for (int i=0; i<text_length; i++)
    {
      // space
      if (text[i] == ' ')
      {
        // translate modelview matrix
        glm_translate_x(shared_font_shaderprogram->model_matrix, font->space);
        // issue update to gpu
        KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);
        render_x += font->space;
      }
      else if (text[i] == '\n')
      {
        // translate modelview matrix
        glm_translate(shared_font_shaderprogram->model_matrix, (vec3){ x - render_x, font->newline, 0.f});
        // issue update to gpu
        KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);
        render_y += font->newline;
        render_x += x - render_x;
      }
      else
      {
        // get ascii
        GLuint ascii = (unsigned char)text[i];

        // draw quad using vertex data and index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ss->index_buffers[ascii]);
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

        // get clip
        RECT* clip = (RECT*)vector_get(ss->clips, ascii);
        // move over
        glm_translate_x(shared_font_shaderprogram->model_matrix, clip->w + BETWEEN_CHAR_SPACING);
        // issue update to gpu
        KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);
        render_x += clip->w + BETWEEN_CHAR_SPACING;
      }
    }
  }
}

void KRR_FONT_render_textex(KRR_FONT* font, const char* text, GLfloat x, GLfloat y, const SIZE* area_size, int align)
{
  // get spritesheet
  KRR_SPRITESHEET* ss = font->spritesheet;
  // get texture id
  GLuint texture_id = ss->ltexture->texture_id;

  // draw position
  GLfloat render_x = x;
  GLfloat render_y = y;

  // if the text needs to be aligned
  if (area_size != NULL)
  {
    // correct empty alignment
    if (align == 0)
    {
      align = KRR_FONT_TEXTALIGNMENT_LEFT | KRR_FONT_TEXTALIGNMENT_TOP;
    }

    // handle horizontal alignment
    if (align & KRR_FONT_TEXTALIGNMENT_CENTERED_H)
    {
      render_x = x + (area_size->w - KRR_FONT_string_width(font, text)) / 2.f;
    }
    else if (align & KRR_FONT_TEXTALIGNMENT_RIGHT)
    {
      render_x = x + area_size->w - KRR_FONT_string_width(font, text);
    }

    // handle vertical alignment
    if (align & KRR_FONT_TEXTALIGNMENT_CENTERED_V)
    {
      render_y = y + (area_size->h - KRR_FONT_string_height(font, text)) / 2.f;
    }
    else if (align & KRR_FONT_TEXTALIGNMENT_BOTTOM)
    {
      render_y = y + area_size->h - KRR_FONT_string_height(font, text);
    }
  }

  // translate to render position
  glm_translate(shared_font_shaderprogram->model_matrix, (vec3){render_x, render_y, 0.f});
  // update modelview matrix immediately
  KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);

  // set texture
  glBindTexture(GL_TEXTURE_2D, texture_id);

  // go through string
  int text_length = strlen(text);
  for (int i=0; i<text_length; i++)
  {
    // space
    if (text[i] == ' ')
    {
      // translate modelview matrix
      glm_translate_x(shared_font_shaderprogram->model_matrix, font->space);
      // immediately issue udpate to gpu
      KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);
      render_x += font->space;
    }
    // newlines
    else if (text[i] == '\n')
    {
      // handle horizontal alignment
      GLfloat target_x = x;

      if (area_size != NULL)
      {
        // handle horizontal alignment
        if (align & KRR_FONT_TEXTALIGNMENT_CENTERED_H)
        {
          target_x += (area_size->w - KRR_FONT_string_width(font, text + i + 1)) / 2.f;
        }
        else if (align & KRR_FONT_TEXTALIGNMENT_RIGHT)
        {
          target_x += area_size->w - KRR_FONT_string_width(font, text + i + 1);
        }
      }
      // translate modelview matrix
      glm_translate(shared_font_shaderprogram->model_matrix, (vec3){target_x - render_x, font->newline, 0.f});
      // issue update to gpu immediately
      KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);
      render_y += font->newline;
      render_x += target_x - render_x;
    }
    else
    {
      // get ascii
      GLuint ascii = (unsigned char)text[i];

      // draw quad using vertex data and index data
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ss->index_buffers[ascii]);
      glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

      // get clip
      RECT* clip = (RECT*)vector_get(ss->clips, ascii);
      // move over
      glm_translate_x(shared_font_shaderprogram->model_matrix, clip->w + BETWEEN_CHAR_SPACING);
      // issue update to gpu
      KRR_FONTSHADERPROG2D_update_model_matrix(shared_font_shaderprogram);
      render_x += clip->w + BETWEEN_CHAR_SPACING;
    }
  }
}

void KRR_FONT_unbind_vao(KRR_FONT* font)
{
  glBindVertexArray(0);
}

GLfloat KRR_FONT_string_width(KRR_FONT* font, const char* string)
{
  GLfloat width = 0.f;

  // go through string
  for (int i=0; string[i] != '\0' && string[i] != '\n'; i++)
  {
    // space
    if (string[i] == ' ')
    {
      width += font->space + BETWEEN_CHAR_SPACING;
    }
    // character
    else
    {
      // get ASCII
      GLuint ascii = (unsigned char)string[i];
      // note: will possibly be bottleneck later as it needs to convert data type here
      // consider has a specific type of vector here later?
      width += (*(RECT*)vector_get(font->spritesheet->clips, ascii)).w + BETWEEN_CHAR_SPACING;
    }
  } 

  return width;
}

GLfloat KRR_FONT_string_height(KRR_FONT* font, const char* string)
{
  GLfloat height = font->line_height;

  // go through string
  for (int i=0; string[i] != '\0'; i++)
  {
    // more space accumulated
    if (string[i] == '\n')
    {
      height += font->line_height;
    }
  }

  return height;
}

SIZE KRR_FONT_get_string_area_size(KRR_FONT* font, const char* text)
{
  // initialize area
  GLfloat sub_width = 0.f;
  SIZE area = {sub_width, font->line_height};

  // go through string
  for (int i=0; i<strlen(text); i++)
  {
    // space
    if (text[i] == ' ')
    {
      sub_width += font->space + BETWEEN_CHAR_SPACING;
    }
    // newline
    else if (text[i] == '\n')
    {
      // add another line
      area.h += font->line_height;

      // check for max width to update max width
      if (sub_width > area.w)
      {
        area.w = sub_width;
        sub_width = 0.f;
      }
    }
    // Character
    else
    {
      // get ascii
      GLuint ascii = (unsigned char)text[i];
      sub_width += (*(RECT*)vector_get(font->spritesheet->clips, ascii)).w + BETWEEN_CHAR_SPACING;
    }
  }

  // check for max width
  if (sub_width > area.w)
  {
    area.w = sub_width;
  }

  return area;
}
