#include "krr/graphics/util.h"
#include <stdarg.h>
#include "krr/foundation/log.h"
#include <SDL2/SDL_image.h>

void KRR_gputil_adapt_to_normal(int screen_width, int screen_height)
{
	// set viewport
	glViewport(0.0, 0.0, screen_width, screen_height);
}

void KRR_gputil_adapt_to_letterbox(int screen_width, int screen_height, int logical_width, int logical_height, int* result_view_width, int* result_view_height, int* offset_x, int* offset_y)
{
  // coordinate is different from OpenGL, Y+ is up, and Y- is down; opposite of opengl
	float aspect = logical_width * 1.0f / logical_height;

	// prioritize base on height of screen's height first
	int view_height = screen_height;
	int view_width = screen_height * aspect;

	// if view's width needs more space to show all game content and cannot fit
	// into current screen's width then we need to do another approach to base
	// on width instead
	// note: less likely to happen as most game would letterbox on the horizontal side not on vertical side
	if (view_width > screen_width)
	{
		view_width = screen_width;
		view_height = screen_width / aspect;
	}

	int viewport_x = (screen_width - view_width) / 2;
	int viewport_y = (screen_height - view_height) / 2;

	// set viewport
  glViewport(viewport_x, viewport_y, view_width, view_height);

	// returning values back from functions via variables
	if (result_view_width != NULL)
	{
		*result_view_width = view_width;
	}
	if (result_view_height != NULL)
	{
		*result_view_height = view_height;
	}
	if (offset_x != NULL)
	{
		*offset_x = viewport_x;
	}
	if (offset_y != NULL)
	{
		*offset_y = viewport_y;
	}
}

// implementation note: decided to return string literal instead of using global string variable
// as to reduce management effort in code to clear error message when execute next related command
const char* KRR_gputil_error_string(GLenum error)
{
  if (error == GL_NO_ERROR)
    return "No error has been recorded. The value of this symbolic constant is guaranteed to be 0.";
  else if (error == GL_INVALID_ENUM)
    return "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
  else if (error == GL_INVALID_VALUE)
    return "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
  else if (error == GL_INVALID_OPERATION)
    return "The specified operation is not allowed in the current state. The offending command is ignored and has no side effect than to set the error flag";
  else if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
    return "The framebuffer object is not complete. The offending command is ignored and has no side effect than to set the error flag.";
  else if (error == GL_OUT_OF_MEMORY)
    return "There is not enough memory left to execute the command";
  else
    return "Unknown error"; // should not happen anyway if you get error from glGetError()
}

GLuint KRR_gputil_map_color_RGBA_to_ABGR(GLuint color)
{
  // map color key to ABGR format
  GLubyte* color_key_bytes = (GLubyte*)&color;
  // we get sequence of bytes which ranging from least to most significant so we can use it right away
  return (color_key_bytes[0] << 24) | (color_key_bytes[1] << 16) | (color_key_bytes[2] << 8) | (color_key_bytes[3]); 
}

GLenum KRR_gputil_anyerror(const char* prefix)
{
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    if (prefix == NULL)
      KRR_LOGE("[] Opengl error code %d: %s", error, KRR_gputil_error_string(error));
    else
      KRR_LOGE("[%s] Opengl error code %d: %s", prefix, error, KRR_gputil_error_string(error));
  }

  return error;
}

void KRR_gputil_update_matrix(GLint location, mat4 matrix)
{
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix[0]);
}

void KRR_gputil_enable_vertex_attrib_pointers(GLint location, ...)
{
  va_list va;
  va_start(va, location);
  
  // operate on all input variables
  while (location != -1)
  {
    glEnableVertexAttribArray(location);
    // proceed to next item
    location = va_arg(va, GLint);
  }

  va_end(va);
}

void KRR_gputil_disable_vertex_attrib_pointers(GLint location, ...)
{
  va_list va;
  va_start(va, location);
  
  // operate on all input variables
  while (location != -1)
  {
    glDisableVertexAttribArray(location);
    // proceed to next item
    location = va_arg(va, GLint);
  }

  va_end(va);
}

void KRR_gputil_create_view_matrix(vec3 trans, vec3 rot, float scale, mat4 dst)
{
  glm_mat4_identity(dst);
  glm_scale_uni(dst, scale);
  glm_rotate(dst, glm_rad(-rot[1]), (vec3){0.f, 1.f, 0.f});
  glm_rotate(dst, glm_rad(-rot[0]), (vec3){1.f, 0.f, 0.f});
  glm_translate(dst, (vec3){-trans[0], -trans[1], -trans[2]});
}

void KRR_gputil_generate_mipmaps(GLenum target, int min_lod, int max_lod)
{
  // always set the maximum index of mipmap level as at the loading time, it usually be set to 0
  // thus this will make genarating mipmaps correctly perform
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(target, GL_TEXTURE_MIN_LOD, min_lod);
  glTexParameterf(target, GL_TEXTURE_MAX_LOD, max_lod);

  glGenerateMipmap(target);
}

int KRR_gputil_load_cubemap(const char* right, const char* left, const char* top, const char* bottom, const char* back, const char* front)
{
  // pre-error checking
  if (right == NULL ||
      left == NULL ||
      top == NULL ||
      bottom == NULL ||
      back == NULL ||
      front == NULL)
  {
    KRR_LOGE("Error: one of textures for cubemap generation is empty");
    return -1;
  }

  // generate texture name for cubemap
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
  
  // set texture parameters
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  // there's no mipmap for this single texture
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

  // create texture for each side of cubemap
  SDL_Surface* temptex = NULL;
  SDL_Surface* ctemptex = NULL;
  // => right
  temptex = IMG_Load(right);
  if (temptex == NULL)
  {
    KRR_LOGE("Unable to load %s! Error: %s", right, IMG_GetError());
    // clear generated texture name
    glDeleteTextures(1, &textureID);
    return -1;
  }
  // convert pixel format
  ctemptex = SDL_ConvertSurfaceFormat(temptex, SDL_PIXELFORMAT_RGB24, 0);
  if (ctemptex == NULL)
  {
    KRR_LOGE("Cannot convert to ABGR8888 format");
    SDL_FreeSurface(temptex);
    
    glDeleteTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return -1;
  }
  SDL_FreeSurface(temptex);
  temptex = NULL;

  // note: lock texture in case RLE compression is applied, but no harm if not
  SDL_LockSurface(ctemptex);
#ifdef GL_SRGB8
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_SRGB8, ctemptex->w, ctemptex->h, 0,  GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#else
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB8, ctemptex->w, ctemptex->h, 0,  GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#endif
  SDL_UnlockSurface(ctemptex);
  // free surface
  SDL_FreeSurface(ctemptex);
  ctemptex = NULL;

  // => left
  temptex = IMG_Load(left);
  if (temptex == NULL)
  {
    KRR_LOGE("Unable to load %s! Error: %s", left, IMG_GetError());
    // clear generated texture name
    glDeleteTextures(1, &textureID);
    return -1;
  }
  // convert pixel format
  ctemptex = SDL_ConvertSurfaceFormat(temptex, SDL_PIXELFORMAT_RGB24, 0);
  if (ctemptex == NULL)
  {
    KRR_LOGE("Cannot convert to ABGR8888 format");
    SDL_FreeSurface(temptex);
    
    glDeleteTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return -1;
  }
  SDL_FreeSurface(temptex);
  temptex = NULL;

  SDL_LockSurface(ctemptex);
#ifdef GL_SRGB8
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_SRGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#else
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#endif
  SDL_UnlockSurface(ctemptex);
  // free surface
  SDL_FreeSurface(ctemptex);
  ctemptex = NULL;

  // => top
  temptex = IMG_Load(top);
  if (temptex == NULL)
  {
    KRR_LOGE("Unable to load %s! Error: %s", top, IMG_GetError());
    // clear generated texture name
    glDeleteTextures(1, &textureID);
    return -1;
  }
  // convert pixel format
  ctemptex = SDL_ConvertSurfaceFormat(temptex, SDL_PIXELFORMAT_RGB24, 0);
  if (ctemptex == NULL)
  {
    KRR_LOGE("Cannot convert to ABGR8888 format");
    SDL_FreeSurface(temptex);
    
    glDeleteTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return -1;
  }
  SDL_FreeSurface(temptex);
  temptex = NULL;
  
  SDL_LockSurface(ctemptex);
#ifdef GL_SRGB8
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_SRGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#else
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#endif
  SDL_UnlockSurface(ctemptex);
  // free surface
  SDL_FreeSurface(ctemptex);
  ctemptex = NULL;

  // => bottom
  temptex = IMG_Load(bottom);
  if (temptex == NULL)
  {
    KRR_LOGE("Unable to load %s! Error: %s", bottom, IMG_GetError());
    // clear generated texture name
    glDeleteTextures(1, &textureID);
    return -1;
  }
  // convert pixel format
  ctemptex = SDL_ConvertSurfaceFormat(temptex, SDL_PIXELFORMAT_RGB24, 0);
  if (ctemptex == NULL)
  {
    KRR_LOGE("Cannot convert to ABGR8888 format");
    SDL_FreeSurface(temptex);
    
    glDeleteTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return -1;
  }
  SDL_FreeSurface(temptex);
  temptex = NULL;

  SDL_LockSurface(ctemptex);
#ifdef GL_SRGB8
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_SRGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#else
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#endif
  SDL_UnlockSurface(ctemptex);
  // free surface
  SDL_FreeSurface(ctemptex);
  ctemptex = NULL;

  // => back
  temptex = IMG_Load(back);
  if (temptex == NULL)
  {
    KRR_LOGE("Unable to load %s! Error: %s", back, IMG_GetError());
    // clear generated texture name
    glDeleteTextures(1, &textureID);
    return -1;
  }
  // convert pixel format
  ctemptex = SDL_ConvertSurfaceFormat(temptex, SDL_PIXELFORMAT_RGB24, 0);
  if (ctemptex == NULL)
  {
    KRR_LOGE("Cannot convert to ABGR8888 format");
    SDL_FreeSurface(temptex);
    
    glDeleteTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return -1;
  }
  SDL_FreeSurface(temptex);
  temptex = NULL;

  SDL_LockSurface(ctemptex);
#ifdef GL_SRGB8
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_SRGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#else
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#endif
  SDL_UnlockSurface(ctemptex);
  // free surface
  SDL_FreeSurface(ctemptex);
  ctemptex = NULL;

  // => front 
  temptex = IMG_Load(front);
  if (temptex == NULL)
  {
    KRR_LOGE("Unable to load %s! Error: %s", front, IMG_GetError());
    // clear generated texture name
    glDeleteTextures(1, &textureID);
    return -1;
  }
  // convert pixel format
  ctemptex = SDL_ConvertSurfaceFormat(temptex, SDL_PIXELFORMAT_RGB24, 0);
  if (ctemptex == NULL)
  {
    KRR_LOGE("Cannot convert to ABGR8888 format");
    SDL_FreeSurface(temptex);
    
    glDeleteTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return -1;
  }
  SDL_FreeSurface(temptex);
  temptex = NULL;

  SDL_LockSurface(ctemptex);
#ifdef GL_SRGB8
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_SRGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#else
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB8, ctemptex->w, ctemptex->h, 0, GL_RGB, GL_UNSIGNED_BYTE, ctemptex->pixels);
#endif
  SDL_UnlockSurface(ctemptex);
  // free surface
  SDL_FreeSurface(ctemptex);
  ctemptex = NULL;

  // unbind cubemap texture
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  return textureID;
}
