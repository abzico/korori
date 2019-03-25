#include "krr/foundation/math.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

float KRR_math_lerp(float a, float b, float t)
{
  return a + (b-a)*t;
}

void KRR_math_lerpv(vec3 a, vec3 b, float t)
{
  // return result in vector a
  a[0] = a[0] + (b[0]-a[0])*t;
  a[1] = a[1] + (b[1]-a[1])*t;
  a[2] = a[2] + (b[2]-a[2])*t;
}

void KRR_math_rand_seed_time(void)
{
  // set seed from current time
  srand(time(NULL));
}

void KRR_math_rand_seed(unsigned int seed)
{
  // set seed from specified seed number
  srand(seed);
}

int KRR_math_rand_int(int max)
{
  // FIXME: this is POOR approach, see http://c-faq.com/lib/randrange.html
  return rand() % (max+1);
}

int KRR_math_rand_int2(int min, int max)
{
  // FIXME: this is POOR approach, see http://c-faq.com/lib/randrange.html
  return rand() % (max+1-min) + min;
}

float KRR_math_rand_float(float max)
{
  return (float)((double)rand() / ((double)RAND_MAX + 1) * (max+1));
}

float KRR_math_rand_float2(float min, float max)
{
  return (float)((double)rand() / ((double)RAND_MAX + 1) * (max+1-min) + min);
}

bool KRR_math_checkCollision(SDL_Rect a, SDL_Rect b, int* deltaCollisionX, int* deltaCollisionY)
{
  if (a.x + a.w > b.x &&
      a.y + a.h > b.y &&
      b.x + b.w > a.x &&
      b.y + b.h > a.y)
  {
    // calculate delta collision distance 
    // calculate based from a to b
    if (deltaCollisionX != NULL) {
      if (a.x + a.w > b.x && a.x + a.w - b.x < a.w/2) {
        *deltaCollisionX = a.x + a.w - b.x;
      }
      // opposite direction
      else if (b.x + b.w > a.x && b.x + b.w - a.x < a.w/2) {
        *deltaCollisionX = (b.x + b.w - a.x) * -1;
      }
    }
    
    // same as above, from a to b
    if (deltaCollisionY != NULL) {
      if (a.y + a.h > b.y && a.y + a.h - b.y < a.h/2) {
        *deltaCollisionY = a.y + a.h - b.y;
      }
      else if (b.y + b.h > a.y && b.y + b.h - a.y < a.h/2) {
        *deltaCollisionY = (b.y + b.h - a.y) * -1;
      }
    }
    return true;
  }
  else
  {
    // just ignore delta collision
    return false;
  }
}

bool KRR_math_checkCollisions(SDL_Rect* collidersA, int numCollidersA, SDL_Rect* collidersB, int numCollidersB, int* deltaCollisionX, int* deltaCollisionY)
{
  SDL_Rect rectA;
  SDL_Rect rectB;

  // go through each box
  for (int i=0; i<numCollidersA; i++)
  {
    rectA = collidersA[i];

    for (int j=0; j<numCollidersB; j++)
    {
      rectB = collidersB[j];

      // not call checkCollision() for better performance
      if (rectA.x + rectA.w > rectB.x &&
          rectA.y + rectA.h > rectB.y &&
          rectB.x + rectB.w > rectA.x &&
          rectB.y + rectB.h > rectA.y)
      {
        // calculate delta collision distance 
        // calculate based from a to b
        if (deltaCollisionX != NULL) {
          if (rectA.x + rectA.w > rectB.x && rectA.x + rectA.w - rectB.x < rectA.w/2) {
            *deltaCollisionX = rectA.x + rectA.w - rectB.x;
          }
          // opposite direction
          else if (rectB.x + rectB.w > rectA.x && rectB.x + rectB.w - rectA.x < rectA.w/2) {
            *deltaCollisionX = (rectB.x + rectB.w - rectA.x) * -1;
          }
        }
        
        // same as above, from a to b
        if (deltaCollisionY != NULL) {
          if (rectA.y + rectA.h > rectB.y && rectA.y + rectA.h - rectB.y < rectA.h/2) {
            *deltaCollisionY = rectA.y + rectA.h - rectB.y;
          }
          else if (rectB.y + rectB.h > rectA.y && rectB.y + rectB.h - rectA.y < rectA.h/2) {
            *deltaCollisionY = (rectB.y + rectB.h - rectA.y) * -1;
          }
        }
        return true;
      } 
    }
  }

  return false;
}

bool KRR_math_checkCollision_cc(CIRCLE a, CIRCLE b, int* deltaCollisionX, int* deltaCollisionY)
{
  if (a.x + a.r > b.x - b.r &&
      a.y + a.r > b.y - b.r &&
      b.x + b.r > a.x - a.r &&
      b.y + b.r > a.y - a.r)
  {
    // calculate delta collision distance 
    // calculate based from a to b
    if (deltaCollisionX != NULL) {
      if (a.x + a.r > b.x - b.r && a.x + a.r - (b.x - b.r) < a.r/2) {
        *deltaCollisionX = a.x + a.r - (b.x - b.r);
      }
      // opposite direction
      else if (b.x + b.r > a.x - a.r && b.x + b.r - (a.x - a.r) < a.r/2) {
        *deltaCollisionX = (b.x + b.r - (a.x - a.r)) * -1;
      }
    }
    
    // same as above, from a to b
    if (deltaCollisionY != NULL) {
      if (a.y + a.r > b.y - b.r && a.y + a.r - (b.y - b.r) < a.r/2) {
        *deltaCollisionY = a.y + a.r - (b.y - b.r);
      }
      // opposite direction
      else if (b.y + b.r > a.y - a.r && b.y + b.r - (a.y - a.r) < a.r/2) {
        *deltaCollisionY = (b.y + b.r - (a.y - a.r)) * -1;
      }
    }
    return true;
  }
  else
  {
    // just ignore delta collision
    return false;
  }
}

bool KRR_math_checkCollision_cr(CIRCLE colliderA, SDL_Rect colliderB, int* deltaCollisionX, int* deltaCollisionY)
{
  // not call checkCollision() for better performance
  if (colliderA.x + colliderA.r > colliderB.x &&
      colliderA.y + colliderA.r > colliderB.y &&
      colliderB.x + colliderB.w > colliderA.x - colliderA.r &&
      colliderB.y + colliderB.h > colliderA.y - colliderA.r)
  {
    // calculate delta collision distance 
    // calculate based from a to b
    if (deltaCollisionX != NULL) {
      if (colliderA.x + colliderA.r > colliderB.x && colliderA.x + colliderA.r - colliderB.x < colliderA.r/2) {
        *deltaCollisionX = colliderA.x + colliderA.r - colliderB.x;
      }
      // opposite direction
      else if (colliderB.x + colliderB.w > colliderA.x - colliderA.r && colliderB.x + colliderB.w - (colliderA.x - colliderA.r) < colliderA.r/2) {
        *deltaCollisionX = (colliderB.x + colliderB.w - (colliderA.x - colliderA.r)) * -1;
      }
    }
    
    // same as above, from a to b
    if (deltaCollisionY != NULL) {
      if (colliderA.y + colliderA.r > colliderB.y && colliderA.y + colliderA.r - colliderB.y < colliderA.r/2) {
        *deltaCollisionY = colliderA.y + colliderA.r - colliderB.y;
      }
      else if (colliderB.y + colliderB.h > colliderA.y - colliderA.r && colliderB.y + colliderB.h - (colliderA.y - colliderA.r) < colliderA.r/2) {
        *deltaCollisionY = (colliderB.y + colliderB.h - (colliderA.y - colliderA.r)) * -1;
      }
    }
    return true;
  } 
  else
  {
    return false;
  }
}

bool KRR_math_checkCollision_cr_arr(CIRCLE colliderA, SDL_Rect* collidersB, int numCollidersB, int* deltaCollisionX, int* deltaCollisionY)
{
  SDL_Rect rectB;

  for (int j=0; j<numCollidersB; j++)
  {
    rectB = collidersB[j];

    // not call checkCollision() for better performance
    if (colliderA.x + colliderA.r > rectB.x &&
        colliderA.y + colliderA.r > rectB.y &&
        rectB.x + rectB.w > colliderA.x - colliderA.r &&
        rectB.y + rectB.h > colliderA.y - colliderA.r)
    {
      // calculate delta collision distance 
      // calculate based from a to b
      if (deltaCollisionX != NULL) {
        if (colliderA.x + colliderA.r > rectB.x && colliderA.x + colliderA.r - rectB.x < colliderA.r/2) {
          *deltaCollisionX = colliderA.x + colliderA.r - rectB.x;
        }
        // opposite direction
        else if (rectB.x + rectB.w > colliderA.x - colliderA.r && rectB.x + rectB.w - (colliderA.x - colliderA.r) < colliderA.r/2) {
          *deltaCollisionX = (rectB.x + rectB.w - (colliderA.x - colliderA.r)) * -1;
        }
      }
      
      // same as above, from a to b
      if (deltaCollisionY != NULL) {
        if (colliderA.y + colliderA.r > rectB.y && colliderA.y + colliderA.r - rectB.y < colliderA.r/2) {
          *deltaCollisionY = colliderA.y + colliderA.r - rectB.y;
        }
        else if (rectB.y + rectB.h > colliderA.y - colliderA.r && rectB.y + rectB.h - (colliderA.y - colliderA.r) < colliderA.r/2) {
          *deltaCollisionY = (rectB.y + rectB.h - (colliderA.y - colliderA.r)) * -1;
        }
      }
      return true;
    } 
  }

  return false;
}

int KRR_math_max(int a, int b)
{
  if (a > b)
    return a;
  else
    return b;
}

int KRR_math_min(int a, int b)
{
  if (a < b)
    return a;
  else
    return b;
}

int KRR_math_bitcount(int value)
{
  int bitcount = 0;

  while (value)
  {
    bitcount++;
    
    // shift bit to the right by 1
    value >>= 1; 
  }

  return bitcount;
}

float KRR_math_barycentric_xz(vec3 p1, vec3 p2, vec3 p3, vec2 p)
{
  float det = (p2[2] - p3[2]) * (p1[0] - p3[0]) + (p3[0] - p2[0]) * (p1[2] - p3[2]);
	float l1 = ((p2[2] - p3[2]) * (p[0] - p3[0]) + (p3[0] - p2[0]) * (p[1] - p3[2])) / det;
	float l2 = ((p3[2] - p1[2]) * (p[0] - p3[0]) + (p1[0] - p3[0]) * (p[1] - p3[2])) / det;
	float l3 = 1.0f - l1 - l2;
	return l1 * p1[1] + l2 * p2[1] + l3 * p3[1];
}

void KRR_math_quat_v2rot(vec3 v1, vec3 v2, versor dest)
{
  // port implementation at http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
  // into C here (use cglm)
  glm_vec3_normalize(v1);
  glm_vec3_normalize(v2);

  float cos_theta = glm_vec3_dot(v1, v2);
  CGLM_ALIGN(8) vec3 rot_axis;

  // handle opposite direction of vectors
  if (cos_theta < -1 + 0.001f)
  {
    // if vectors are in opposite direction, then use just one vector which need to be
    // perpendicular to v1 vector
    glm_vec3_cross(GLM_ZUP, v1, rot_axis);
    if (glm_vec3_dot(rot_axis, rot_axis) < 0.01f) // it's parallell, then use another axis
      glm_vec3_cross(GLM_XUP, v1, rot_axis);

    glm_vec3_normalize(rot_axis);
    // store result into dest (via axis-angle through cglm's glm_quatv())
    glm_quatv(dest, glm_rad(180.0f), rot_axis);
  }
  else
  {
    glm_vec3_cross(v1, v2, rot_axis);
    double s = sqrt((1+cos_theta)*2);
    double invs = 1.0 / s; 

    // store raw results to form quaternion
    glm_quat_init(dest,
        (float)(rot_axis[0] * invs), 
        (float)(rot_axis[1] * invs),
        (float)(rot_axis[2] * invs),
        (float)(s * 0.5));
  }
}
