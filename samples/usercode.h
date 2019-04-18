///
/// User can implement their own user's code for game logic and rendering in this file.
/// It can include either OpenGL and normal SDL2 stuff.
///
///
#ifndef KRR_TEST_USERCODE_h_
#define KRR_TEST_USERCODE_h_

#include <SDL2/SDL.h>
#include "krr/graphics/common.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// Initialize
///
/// User should call this function to initialize with screen's dimension
/// as created externally first.
///
/// \param screen_width Screen width in pixel
/// \param screen_height Screen height in pixel
/// \param logical_width Logical width of game screen logic
/// \param logical_height Logical height of game screen logic
/// \return True if initialize successfully, otherwise return false.
///
extern bool usercode_init(int screen_width, int screen_height, int logical_width, int logical_height);

///
/// Load media
///
/// \return True if loading is successful, otherwise return false.
///
extern bool usercode_loadmedia(void);

///
/// Set screen's dimensions.
///
/// \param window_id Window id that wants to change screen's dimensions (unused for now)
/// \param screen_width Screen width to chagne to
/// \param screen_height Screen height to change to
///
extern void usercode_set_screen_dimension(Uint32 window_id, int screen_width, int screen_height);

///
/// Update
///
/// \param delta_time Elapsed time since last frame in ms.
///
extern void usercode_update(float delta_time);

///
/// Render
///
extern void usercode_render(void);

///
/// Render ui text
///
extern void usercode_render_ui_text(void);

///
/// Render average frame per second
///
/// \param avg_fps average frame per second
///
extern void usercode_render_fps(int avg_fps);

///
/// Handle event.
///
/// \param e SDL_Event for incoming event
/// \param delta_time Elapsed time since last frame in ms.
///
extern void usercode_handle_event(SDL_Event *e, float delta_time);

///
/// Free all resource
///
extern void usercode_close(void);

#ifdef __cplusplus
extern "C" {
#endif

#endif
