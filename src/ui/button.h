/*
 * KRR_BUTTON
 * Represents a *logical* button with 4 possible states of visual which are
 * 	- mouse out
 * 	- mouse over
 * 	- mouse down
 * 	- mouse up
 *
 * 	It works with LTexture which will represent its visual.
 */

#ifndef KRR_BUTTON_h_
#define KRR_BUTTON_h_

#include "SDL.h"

/// state of button
enum KRR_BUTTON_STATE
{
	KRR_BUTTON_MOUSE_OUT = 0,
	KRR_BUTTON_MOUSE_OVER_MOTION = 1,
	KRR_BUTTON_MOUSE_DOWN = 2,
	KRR_BUTTON_MOUSE_UP = 3,
	KRR_BUTTON_TOTAL = 4
};

typedef struct
{
	enum KRR_BUTTON_STATE state;
} KRR_BUTTON;

/*
 * Create a new KRR_BUTTON then return its pointer via out pointer.
 * 
 * Return newly created KRR_BUTTON in which caller needs to free it with KRR_BUTTON_Free().
 */
extern KRR_BUTTON* KRR_BUTTON_create();

/*
 * Call this function to update KRR_BUTTON's state, and execute user's custon handler function as set via its structure.
 *
 * User calls this in application's event loop.
 * Parameter
 * 	- button, KRR_BUTTON* : pointer to KRR_BUTTON
 * 	- e, SDL_Event* : pointer to SDL_Event as received in user's event loop code
 * 	- pos, SDL_Point : position that this button will be drawed
 * 	- buttonRect, SDL_Rect : rectangle defined the coordinate to render such KRR_BUTTON
 */
extern void KRR_BUTTON_handleEvent(KRR_BUTTON* button, SDL_Event* e, SDL_Rect buttonRect);

/*
 * Free KRR_BUTTON.
 * After calling this function, button will be nil.
 * button cannot be nil.
 *
 * Paramter
 * 	- button KRR_BUTTON* : pointer to KRR_BUTTON to free
 */
extern void KRR_BUTTON_free(KRR_BUTTON* button);

#endif 	/* KRR_BUTTON_h_ */

