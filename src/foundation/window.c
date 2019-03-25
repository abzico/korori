#include "krr/foundation/window.h"
#include <SDL2/SDL_log.h>
#include <stdlib.h>

#define WINDOW_TITLE_BUFFER 60
static void initialize_values(KRR_WINDOW* window)
{
  window->window = NULL;
  window->renderer = NULL;
  window->has_mouse_focus = false;
  window->has_keyboard_focus = false;
  window->is_minimized = false;
  window->is_shown = false;
  window->fullscreen = false;
  window->width = 0;
  window->height = 0;
  memset(window->original_title, 0, ORIGINAL_WINDOW_TITLE_BUFFER);
  window->on_window_resize = NULL;
  window->handle_event = NULL;
  window->render = NULL;
}

KRR_WINDOW* KRR_WINDOW_new(const char* title, int screen_width, int screen_height, int window_flags, int renderer_flags)
{
  // allocate memory
  KRR_WINDOW* out = malloc(sizeof(KRR_WINDOW));
  
  // initially set values
  initialize_values(out);

  // initialize
  if(!KRR_WINDOW_init(out, title, screen_width, screen_height, window_flags, renderer_flags))
  {
    // if failed to init, then free allocated memory
    KRR_WINDOW_free(out);
    return NULL;
  }
  else
  {
    return out;
  }
}

bool KRR_WINDOW_init(KRR_WINDOW *window, const char* title, int screen_width, int screen_height, int window_flags, int renderer_flags)
{
  // not always assume to set SDL_WINDOW_SHOWN all the time, in case of headless we do need
  // SDL_WINDOW_HIDDEN
  window->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, window_flags);
  if (window->window != NULL)
  {
    window->has_mouse_focus = true;
    window->has_keyboard_focus = true;
    window->width = screen_width;
    window->height = screen_height;
    strncpy(window->original_title, title, ORIGINAL_WINDOW_TITLE_BUFFER);

    // check if there is opengl bit set then we don't create renderer
    if (!(window_flags & SDL_WINDOW_OPENGL))
    {
      // create renderer associates with this window
      // default use hardware acceleration
      window->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED | renderer_flags);
      // if cannot create renderer, then we free window immediately
      if (window->renderer == NULL)
      {
        // free internals
        KRR_WINDOW_free_internals(window);
        return false;
      }
    }

		// initially set fullscreen flag if immediately set window to show in full screen
		if ((window_flags & SDL_WINDOW_FULLSCREEN) || (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP))
		{
			window->fullscreen = true;
			window->is_minimized = false;
		}
    
    // grab window identifier
    window->id = SDL_GetWindowID(window->window);
    // grab display index that this window is on
    window->display_id = SDL_GetWindowDisplayIndex(window->window);
    // set this window as opened
    window->is_shown = true;
  }

  // return result of initialization
  return true;
}

void KRR_WINDOW_handle_event(KRR_WINDOW* window, SDL_Event *e, float delta_time)
{
  // window event occurred
  if (e->type == SDL_WINDOWEVENT && e->window.windowID == window->id)
  {
    switch (e->window.event)
    {
      // window moved
      case SDL_WINDOWEVENT_MOVED:
        window->display_id = SDL_GetWindowDisplayIndex(window->window);
        break;

      // window appeared
      case SDL_WINDOWEVENT_SHOWN:
        window->is_shown = true;
        break;
        
      // window disappeared
      case SDL_WINDOWEVENT_HIDDEN:
        window->is_shown = false;
        break;

      // get new dimensions and repaint on window size change
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        window->width = e->window.data1;
        window->height = e->window.data2;

        // notify event via function pointer (if it's set)
        if (window->on_window_resize != NULL)
        {
          window->on_window_resize(window->id, window->width, window->height);
        }

        SDL_RenderPresent(window->renderer);
        break;

      // repaint on exposure
      case SDL_WINDOWEVENT_EXPOSED:
        SDL_RenderPresent(window->renderer);
        break;

      // mouse entered window
      case SDL_WINDOWEVENT_ENTER:
        window->has_mouse_focus = true;
        break;

      // mouse left window
      case SDL_WINDOWEVENT_LEAVE:
        window->has_mouse_focus = false;
        break;

      // window has keyboard focus
      case SDL_WINDOWEVENT_FOCUS_GAINED:
        window->has_keyboard_focus = true;

        if (window->on_window_focus_gained != NULL)
        {
          window->on_window_focus_gained(window->id);
        }
        break;

      // window lost keyboard focus
      case SDL_WINDOWEVENT_FOCUS_LOST:
        window->has_keyboard_focus = false;

        if (window->on_window_focus_lost != NULL)
        {
          window->on_window_focus_lost(window->id);
        }
        break;

      // window minimized
      case SDL_WINDOWEVENT_MINIMIZED:
        window->is_minimized = true;
        break;

      // window maximized
      case SDL_WINDOWEVENT_MAXIMIZED:
        window->is_minimized = false;
        break;

      // windnow restored
      case SDL_WINDOWEVENT_RESTORED:
        window->is_minimized = false;
        break;

      // hide on close
      case SDL_WINDOWEVENT_CLOSE:
        SDL_HideWindow(window->window);
        break;
    }
  }

  // execute user's code on handling event if set
  if (window->handle_event != NULL)
  {
    window->handle_event(e, delta_time);
  }
}

void KRR_WINDOW_set_fullscreen(KRR_WINDOW *window, bool fullscreen)
{
	if (window->fullscreen)
	{
		// 0 value for windowed mode
		SDL_SetWindowFullscreen(window->window, 0);
		window->fullscreen = false;
	}
	else
	{
		// SDL_WINDOW_FULLSCREEN_DESKTOP for "fake" fullscreen without changing videomode
		// depends on type of game, and performance aim i.e. FPS game might want to do "real" fullscreen
		// by changing videomode to get performance gain, but point and click with top-down tile-based
		// might not need to change videomode to match the desire spec.
		//
		// as well this needs to work with SDL_RenderSetLogicalSize() function to make it works.
		SDL_SetWindowFullscreen(window->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		window->fullscreen = true;
		window->is_minimized = false;
	}

	// get new width and height of window
	int w, h;
	SDL_GetWindowSize(window->window, &w, &h);
	// update width and height of window
	//window->width = w;
	//window->height = h;
	// trigger winsize event changed (this won't be in event supported by SDL2)
	if (window->on_window_resize != NULL)
	{
		window->on_window_resize(window->id, w, h);
	}
}

void KRR_WINDOW_focus(KRR_WINDOW* window)
{
  // restore window if needed
  if (window->is_shown)
  {
    SDL_ShowWindow(window->window);
  }

  // move window forward
  SDL_RaiseWindow(window->window);
}

void KRR_WINDOW_free_internals(KRR_WINDOW* window)
{
  // free associated renderer first
  if (window->renderer != NULL)
  {
    SDL_DestroyRenderer(window->renderer);
    window->renderer = NULL;
  }

  // free window next
  if (window->window != NULL)
  {
    SDL_DestroyWindow(window->window);
    window->window =  NULL;
  }
}

void KRR_WINDOW_free(KRR_WINDOW* window)
{
  // firstly free its internals
  KRR_WINDOW_free_internals(window);
  // free its allocated space
  free(window);
  window = NULL;
}
