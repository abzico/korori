#ifndef krr_TIMER_h_
#define krr_TIMER_h_

#include "SDL.h"
#include <stdbool.h>

typedef struct krr_TIMER {
  // clock time when timer started
  Uint32 started_ticks;

  // clock time when timer paused
  Uint32 paused_ticks;

  // states of timer
  bool paused;
  bool started;
} krr_TIMER;

/*
 * \brief Create a new timer.
 * \return Return a newly created krr_TIMER*
 */
extern krr_TIMER* krr_TIMER_createNew();

/*
 * \brief Start the timer.
 * \param timer A timer to start
 */
extern void krr_TIMER_start(krr_TIMER* timer);

/*
 * \brief Stop the timer.
 * \param timer A timer to stop
 */
extern void krr_TIMER_stop(krr_TIMER* timer);

/*
 * \brief Pause the timer.
 * \param timer A timer to pause.
 */
extern void krr_TIMER_pause(krr_TIMER* timer);

/*
 * \brief Resume the paused timer.
 * \param timer A timer to resume.
 */
extern void krr_TIMER_resume(krr_TIMER* timer);

/*
 * \brief Get current ticks of timer in ms.
 * \param timer A timer to get ticks from
 */
extern Uint32 krr_TIMER_getTicks(krr_TIMER* timer);

/*
 * \brief Free the timer.
 * \param timer A timer to free from memory.
 *
 * It will set NULL to timer after successfully free it, otherwise it won't.
 */
extern void krr_TIMER_free(krr_TIMER* timer);

#endif /* krr_TIMER_h_ */
