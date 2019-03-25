#ifndef KRR_CAM_h_
#define KRR_CAM_h_

#include "krr/graphics/common.h"

typedef struct
{
  /// current frame position
  vec3 pos;
  /// target position to go to for this frame
  vec3 topos;

  /// current rotation angle in degrees
  vec3 rot;
  /// target rotation to rotate to for this frame
  vec3 torot;

  /// forward vector
  vec3 forward;
  /// up vector
  vec3 up;
} KRR_CAM;

#endif
