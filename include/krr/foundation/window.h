#ifndef KRR_window_h_
#define KRR_window_h_

#include <SDL2/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ORIGINAL_WINDOW_TITLE_BUFFER 50

///
/// Wrapper window managmenet for SDL_Window
/// 
typedef struct {
  /// id represents this window
  Uint32 id;
  
  // display id that this window is on
  int display_id;

  /// window
  SDL_Window* window;

  /// renderer associates with this window
  SDL_Renderer* renderer;

  /// window width (read-only)
  int width;

  /// window height (read_only)
  int height;

  /// whether or not has mouse focus.
  /// internally managed, (read-only)
  bool has_mouse_focus;

  /// whether or not has keyboard focus
  /// internally managed (read-only)
  bool has_keyboard_focus;

  /// whether or not the window is minimized
  /// internally managed (read-only
  bool is_minimized;

  /// whether the window is shown
  bool is_shown;

  /// original title of window
  char original_title[ORIGINAL_WINDOW_TITLE_BUFFER];

  /// flag whether it's full screen or not.
  /// (read-only)
  bool fullscreen;

  /// callback available to be set when window's size has changed
  /// This will be called before repainting occurs.
  ///
  /// \param window_id Window id for this callback
  /// \param new_width New window's width
  /// \param new_height New window/s height
  void (*on_window_resize)(Uint32 window_id, int new_width, int new_height);

  /// callback available to be set when window has gained keyboard focus
  /// this can be used especially to slow down framerate when app is not active,
  /// to reduce CPU usage.
  ///
  /// \param window_id window id
  ///
  void (*on_window_focus_gained)(Uint32 window_id);

  /// callback available to be set when window has lost keyboard focus.
  /// this can be used especially to set framerate back to normal when app gains back active.
  ///
  /// \param window_id window id
  ///
  void (*on_window_focus_lost)(Uint32 window_id);

  /// handle evant callback that can be set.
  /// This function will be called after internal handling event is carried out.
  ///
  /// It's optional to use this.
  void (*handle_event)(SDL_Event* e, float delta_time);

  /// render callback function for this window
  /// Currently it's not used internally just yet, but it will make code cleaner in user's code.
  ///
  /// It's optional to use this.
  void (*render)(float delta_time);
} KRR_WINDOW;

///
/// Create a new KRR_WINDOW.
/// Use KRR_WINDOW_free() to free KRR_WINDOW later.
/// \param title Title of window
/// \param screen_width Screen width
/// \param screen_height Screen height
/// \param window_flags Additional flags to set for window. It will logical OR with SDL_WINDOW_SHOWN. Use 0 if no additional flags to be set.
/// \param renderer_flags Additional flags to set for renderer. It will logical OR with SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC. Use 0 if no additional flags to be set.
/// \return Newly created KRR_WINDOW instance on heap.
///
extern KRR_WINDOW* KRR_WINDOW_new(const char* title, int screen_width, int screen_height, int window_flags, int renderer_flags);

///
/// Initialize allocated KRR_WINDOW.
/// Use KRR_WINDOW_free_internals() to free KRR_WINDOW created via this function.
/// \param [in,out] window Window instance
/// \param title Title of window
/// \param screen_width Screen width
/// \param screen_height Screen height
/// \param window_flags Flags to set for window. It will logical OR with SDL_WINDOW_SHOWN. Use 0 if no additional flags to be set.
/// \param renderer_flags Additional flags to set for renderer. It will logical OR with SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC. Use 0 if no additional flags to be set.
/// \return True if initialize successfully, otherwise return false.
/// 
extern bool KRR_WINDOW_init(KRR_WINDOW *window, const char* title, int screen_width, int screen_height, int window_flags, int renderer_flags);

///
/// Handle event associated with window.
/// \param window Window instance
/// \param e SDL_Event to handle
/// \param delta_time Delta time since last frame
///
extern void KRR_WINDOW_handle_event(KRR_WINDOW *window, SDL_Event *e, float delta_time);

///
/// Set window to fullscreen or windowed mode.
///
/// \param window Window instance
/// \param fullscreen True to set to full screen, otherwise false to set to windowed mode
///
extern void KRR_WINDOW_set_fullscreen(KRR_WINDOW* window, bool fullscreen);

///
/// Make window focused
/// \param window Window to make focused
///
extern void KRR_WINDOW_focus(KRR_WINDOW* window);

///
/// Free internals of KRR_WINDOW.
/// Use this to free memory created by KRR_WINDOW via KRR_WINDOW_init().
///
extern void KRR_WINDOW_free_internals(KRR_WINDOW *window);

/// Free memory of KRR_WINDOW.
/// Use this to free memory created by KRR_WINDOW via KRR_WINDOW_new().
///
extern void KRR_WINDOW_free(KRR_WINDOW *window);

#ifdef __cplusplus
}
#endif

#endif
