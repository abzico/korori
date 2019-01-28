#ifndef KRR_math_h_
#define KRR_math_h_

#include "SDL.h"
#include "types.h"
#include "foundation/common.h"

extern float KRR_math_lerp(float a, float b, float t);

/// lerp vector
/// vector a is in,out
extern void KRR_math_lerpv(vec3 a, vec3 b, float t);

/// Seed rand() function with current time via time(NULL).
extern void KRR_math_rand_seed_time(void);

/// Seed rand() with seed
/// \param seed Number to be set as seed for rand()
extern void KRR_math_rand_seed(unsigned int seed);

/// Random integer from [0, max]).
/// \param max Maximum number 
/// \return Randomized number in range [0, max]
extern int KRR_math_rand_int(int max);

/// Random integer from [min, max].
/// \param inclusive_min Minimum number for result
/// \param exclusive_max Maximum number for result
/// \return Randomized number in range [min, max].
extern int KRR_math_rand_int2(int min, int max);

/// Random float number from [0, max].
/// \param max Maximum float number
/// \return Randomized float number in range [0, max].
extern float KRR_math_rand_float(float max);

/// Radom float number from [min, max].
/// \param min Minimum number for result
/// \param max Maximum number for result
/// \return Randomized number in range [min, max].
extern float KRR_math_rand_float2(float min, float max);

extern bool KRR_math_checkCollision(SDL_Rect a, SDL_Rect b, int* deltaCollisionX, int* deltaCollisionY);

extern bool KRR_math_checkCollisions(SDL_Rect *collidersA, int numCollidersA, SDL_Rect* collidersB, int numCollidersB, int* deltaCollisionX, int* deltaCollisionY);

extern bool KRR_math_checkCollision_cc(CIRCLE a, CIRCLE b, int* deltaCollisionX, int* deltaCollisionY);

extern bool KRR_math_checkCollision_cr(CIRCLE colliderA, SDL_Rect colliderB, int* deltaCollisionX, int* deltaCollisionY);

extern bool KRR_math_checkCollision_cr_arr(CIRCLE colliderA, SDL_Rect* collidersB, int numCollidersB, int* deltaCollisionX, int* deltaCollisionY);

///
/// Find max number from two numbers.
///
/// \param a First number
/// \param b Second number
/// \return Maximum number from a and b
///
extern int KRR_math_max(int a, int b);

///
/// Find minimum number from input pair of numbers.
///
/// \param a First number
/// \param b Second number
/// \return Minimum number from a and b
///
extern int KRR_math_min(int a, int b);

///
/// Count number of bit representing the input number
///
/// \pram value Input number to find number of bits
/// \return Number of bits representing input number
///
extern int KRR_math_bitcount(int value);

#endif
