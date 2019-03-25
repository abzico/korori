/**
 * Korori Main Source File
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "krr/foundation/common.h"
#include "krr/foundation/window.h"
#include "krr/graphics/common.h"
#include "krr/graphics/texture.h"
#include "krr/graphics/spritesheet.h"
#include "krr/graphics/font.h"

#include "usercode.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define LOGICAL_WIDTH 640
#define LOGICAL_HEIGHT 480
#define SETFRAME(var, arg1, arg2, arg3, arg4)		\
  do {										\
    var.x = arg1;							\
    var.y = arg2;							\
    var.w = arg3;							\
    var.h = arg4;							\
  } while(0)

// cap thus using fixed deltaTime step
#define FIXED_UPDATERATE .01666666666666666666
// slow down fps when app doesn't have focus
#define COLDSTATE_UPDATERATE 0.1

// -- functions
bool init();
bool setup();
void handleEvent(SDL_Event *e, float deltaTime);
void update(float deltaTime);
void render();
void close();

static void on_window_focus_gained(Uint32 window_id);
static void on_window_focus_lost(Uint32 window_id);

// opengl context
SDL_GLContext opengl_context;

// -- variables
bool quit = false;
static bool app_active = true;
static double active_updaterate = FIXED_UPDATERATE;

// independent time loop
Uint32 currTime = 0;
Uint32 prevTime = 0;

void on_window_focus_gained(Uint32 window_id)
{
  if (window_id == gWindow->id)
  {
    // change target framerate back to normal
    active_updaterate = FIXED_UPDATERATE;
    app_active = true;
  }
}

void on_window_focus_lost(Uint32 window_id)
{
  if (window_id == gWindow->id)
  {
    // slow down framerate
    active_updaterate = COLDSTATE_UPDATERATE;
    app_active = false;
  }
}

bool init() {
  // initialize sdl
  // also init SDL_INIT_JOYSTICK to prevent frame from taken too long than should, see
  // https://discourse.libsdl.org/t/unstable-frame-rate-unexpectedly/25783
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    KRR_LOGE("SDL could not initialize! SDL_Error: %s", SDL_GetError());
    return false;
  }
  
  // use core profile of opengl 3.3
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  // just to be pragmatic
  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
  // following these 3 lines might not be needed
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // preprocessor check for sample for 'headless' sample only
#ifdef SDL_HEADLESS
  // create window headless
  gWindow = KRR_WINDOW_new("Korori - Test", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL, 0);
#else
  // create window normally
  gWindow = KRR_WINDOW_new("Korori - Test", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL, 0);
#endif
  if (gWindow == NULL) {
    KRR_LOGE("Window could not be created! SDL_Error: %s", SDL_GetError());
    return false;
  }
	// listen to window's events
	gWindow->on_window_resize = usercode_set_screen_dimension;
  gWindow->on_window_focus_gained = on_window_focus_gained;
  gWindow->on_window_focus_lost = on_window_focus_lost;

  // create opengl context
  opengl_context = SDL_GL_CreateContext(gWindow->window);
  if (opengl_context == NULL)
  {
    KRR_LOGE("OpenGL context could not be created: %s", SDL_GetError());
    return false;
  }

  // check opengl version we got
  KRR_LOG("OpenGL version %s\nGLSL version: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

  // use vsync
  // we need to enable vsync to resolve stuttering issue for now
  if (SDL_GL_SetSwapInterval(1) != 0)
  {
    KRR_LOGW("Warning: Unable to enable vsync! %s", SDL_GetError());
  }

  // init glew
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK)
  {
    KRR_LOGE("Failed initialize glew! %s", glewGetErrorString(glewError));
    return false;
  }

  // make sure OpenGL 3.3 is supported
  if (!GLEW_VERSION_3_3)
  {
    SDL_Log("OpenGL 3.3 not supported!");
    return false;
  }

  // relay call to user's code in separate file
  if (!usercode_init(SCREEN_WIDTH, SCREEN_HEIGHT, LOGICAL_WIDTH, LOGICAL_HEIGHT))
  {
    SDL_Log("Failed to initialize user's code initializing function");
    return false;
  }

  // initialize png loading
  // see https://www.libsdl.org/projects/SDL_image/docs/SDL_image.html#SEC8
  // returned from IMG_Init is all flags initted, so we could check for all possible
  // flags we aim for
  int imgFlags = IMG_INIT_PNG;
  int inittedFlags = IMG_Init(imgFlags);
  if ( (inittedFlags & imgFlags) != imgFlags)
  {
    // from document, not always that error string from IMG_GetError() will be set
    // so don't depend on it, just for pure information
    SDL_Log("SDL_Image could not initialize! SDL_image Error: %s", IMG_GetError());
    return false;
  }

  return true;
}

// include any asset loading sequence, and preparation code here
bool setup()
{
  // load media from usercode
  if (!usercode_loadmedia())
  {
    SDL_Log("Failed to load media from usercode");
    return false;
  }

  return true;
}

void handleEvent(SDL_Event *e, float deltaTime)
{
  // user requests quit
  if (e->type == SDL_QUIT ||
      (e->key.keysym.sym == SDLK_ESCAPE))
  {
    quit = true;
  }
  else
  {
    // relay call to user's code in separate file
    usercode_handle_event(e, deltaTime);
  }
}

void update(float deltaTime)
{
  // relay call to user's code in separate file
  usercode_update(deltaTime);
}

void render()
{
  if (!gWindow->is_minimized)
  {
    // relay call to user's code in separate file
    usercode_render();
    usercode_render_ui_text();

#ifndef DISABLE_FPS_CALC
    // render fps
    usercode_render_fps((int)lroundf(common_avgFPS));
#endif
  }
}

void close()
{
  // relay call to user's code in separate file
  usercode_close();

	// destroy opengl context
	if (opengl_context != NULL)
	{
		SDL_GL_DeleteContext(opengl_context);
	}

  // destroy window
  KRR_WINDOW_free(gWindow);

  IMG_Quit();
  SDL_Quit();
}

int main(int argc, char* args[])
{
  // start up SDL and create window
  if (!init())
  {
    SDL_Log("Failed to initialize");
  }	
  else
  {
    // load media, and set up
    if (!setup())
    {
      SDL_Log("Failed to setup!");
    }
    else
    {
      // event handler
      SDL_Event e;

      // while application is running
      while (!quit)
      {
        // prepare delta time to feed to both handleEvent(), update(), and render()
        prevTime = currTime;
        currTime = SDL_GetTicks();
        // calculate per second
        double deltaTime = (currTime - prevTime) / 1000.0;

        // fixed step
        common_frameTime += deltaTime;

#ifndef DISABLE_FPS_CALC
				// update accumulated time for calculating framerate
        common_frameAccumTime += deltaTime;
#endif
        if (common_frameTime >= active_updaterate)
        {
#ifndef DISABLE_FPS_CALC
          common_frameCount++;

          // check to reset frame time
          if (common_frameAccumTime >= 1.0)
          {
            common_avgFPS = common_frameCount / common_frameAccumTime;
            common_frameCount = 0;
            common_frameAccumTime = 0.0;
          }
#endif

          // catch up updating time
          while ( common_frameTime >= active_updaterate )
          {
            // handle events on queue
            // if it's 0, then it has no pending event
            // we keep polling all event in each game loop until there is no more pending one left
            while (SDL_PollEvent(&e) != 0)
            {
              // handle window's events
              KRR_WINDOW_handle_event(gWindow, &e, common_frameTime);
              // update user's handleEvent()
              handleEvent(&e, common_frameTime);
            }

            update(active_updaterate);

            // left over will be carried on to the next frame
            common_frameTime -= active_updaterate;
          }

          render();

          // update screen
          SDL_GL_SwapWindow(gWindow->window);
        }

        // eat less CPU cycles
        if (!app_active)
        {
          SDL_Delay(COLDSTATE_UPDATERATE * 1000.0);
        }
      }
    }
  }

  // free resource and close SDL
  close();

  return 0;
}

