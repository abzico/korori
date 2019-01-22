/*
 * krr_BUTTON
 * Represents a *logical* button with 4 possible states of visual which are
 * 	- mouse out
 * 	- mouse over
 * 	- mouse down
 * 	- mouse up
 *
 * 	It works with LTexture which will represent its visual.
 */

#ifndef krr_BUTTON_h_
#define krr_BUTTON_h_

#include "SDL.h"

/// state of button
enum krr_BUTTON_state
{
	BUTTON_MOUSE_OUT = 0,
	BUTTON_MOUSE_OVER_MOTION = 1,
	BUTTON_MOUSE_DOWN = 2,
	BUTTON_MOUSE_UP = 3,
	BUTTON_TOTAL = 4
};

typedef struct
{
	enum krr_BUTTON_state state;
} krr_BUTTON;

/*
 * Create a new krr_BUTTON then return its pointer via out pointer.
 * 
 * Return newly created krr_BUTTON in which caller needs to free it with krr_BUTTON_Free().
 */
extern krr_BUTTON* krr_BUTTON_create();

/*
 * Call this function to update krr_BUTTON's state, and execute user's custon handler function as set via its structure.
 *
 * User calls this in application's event loop.
 * Parameter
 * 	- button, krr_BUTTON* : pointer to krr_BUTTON
 * 	- e, SDL_Event* : pointer to SDL_Event as received in user's event loop code
 * 	- pos, SDL_Point : position that this button will be drawed
 * 	- buttonRect, SDL_Rect : rectangle defined the coordinate to render such krr_BUTTON
 */
extern void krr_BUTTON_handleEvent(krr_BUTTON* button, SDL_Event* e, SDL_Rect buttonRect);

/*
 * Free krr_BUTTON.
 * After calling this function, button will be nil.
 * button cannot be nil.
 *
 * Paramter
 * 	- button krr_BUTTON* : pointer to krr_BUTTON to free
 */
extern void krr_BUTTON_free(krr_BUTTON* button);

#endif 	/* krr_BUTTON_h_ */

