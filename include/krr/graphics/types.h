#ifndef KRR_graphics_types_h_
#define KRR_graphics_types_h_

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__MINGW64__)
#include <GL/gl.h>
#elif defined(__APPLE__) || defined(__MACH__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat w;
  GLfloat h;
} RECT;

typedef struct
{
  GLfloat w;
  GLfloat h;
} SIZE;

typedef struct
{
  GLfloat s;
  GLfloat t;
} TEXCOORD2D;

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat z;
} NORMAL;

typedef struct
{
  GLfloat x;
  GLfloat y;
} VERTEXPOS2D;

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat z;
} VERTEXPOS3D;

typedef struct
{
  VERTEXPOS2D position;
  TEXCOORD2D texcoord;
} VERTEXTEX2D;

typedef struct
{
  VERTEXPOS3D position;
  TEXCOORD2D texcoord;
} VERTEXTEX3D;

typedef struct
{
  VERTEXPOS3D position;
  NORMAL normal;
} VERTEXNORM3D;

typedef struct
{
  VERTEXPOS3D position;
  TEXCOORD2D texcoord;
  NORMAL normal;
} VERTEXTEXNORM3D;

typedef struct
{
  GLfloat r;
  GLfloat g;
  GLfloat b;
} COLOR3F;

typedef struct
{
  GLfloat r;
  GLfloat g;
  GLfloat b;
  GLfloat a;
} COLOR32;

typedef COLOR32 COLOR4F;

typedef struct
{
  VERTEXPOS2D pos;
  COLOR32 color;
} MULTCOLOR2D;

typedef struct
{
  VERTEXPOS3D pos;
  COLOR3F color;
} LIGHT;

typedef struct
{
  GLfloat shine_dumper;
  GLfloat reflectvity;
} MATERIAL;

#endif
