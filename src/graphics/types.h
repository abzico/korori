#ifndef KRR_graphics_types_h_
#define KRR_graphics_types_h_

#include "gl.h"

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
  GLfloat a;
} COLOR32;

typedef struct
{
  VERTEXPOS2D pos;
  COLOR32 color;
} MULTCOLOR2D;

#endif
