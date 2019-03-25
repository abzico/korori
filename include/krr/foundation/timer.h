#ifndef KRR_TIMER_h_
#define KRR_TIMER_h_

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct KRR_TIMER {
  // clock time when timer started
  Uint32 started_ticks;

  // clock time when timer paused
  Uint32 paused_ticks;

  // states of timer
  bool paused;
  bool started;
} KRR_TIMER;

/*
 * \brief Create a new timer.
 * \return Return a newly created KRR_TIMER*
 */
extern KRR_TIMER* KRR_TIMER_createNew(void);

/*
 * \brief Start the timer.
 * \param timer A timer to start
 */
extern void KRR_TIMER_start(KRR_TIMER* timer);

/*
 * \brief Stop the timer.
 * \param timer A timer to stop
 */
extern void KRR_TIMER_stop(KRR_TIMER* timer);

/*
 * \brief Pause the timer.
 * \param timer A timer to pause.
 */
extern void KRR_TIMER_pause(KRR_TIMER* timer);

/*
 * \brief Resume the paused timer.
 * \param timer A timer to resume.
 */
extern void KRR_TIMER_resume(KRR_TIMER* timer);

/*
 * \brief Get current ticks of timer in ms.
 * \param timer A timer to get ticks from
 */
extern Uint32 KRR_TIMER_getTicks(KRR_TIMER* timer);

/*
 * \brief Free the timer.
 * \param timer A timer to free from memory.
 *
 * It will set NULL to timer after successfully free it, otherwise it won't.
 */
extern void KRR_TIMER_free(KRR_TIMER* timer);

#endif
