#include "timer.h"
#include <stdlib.h>

krr_TIMER* krr_TIMER_createNew()
{
  krr_TIMER *timer = malloc(sizeof(krr_TIMER));

  timer->started_ticks = 0;
  timer->paused_ticks = 0;
  timer->paused = false;
  timer->started = false;

  return timer;
}

void krr_TIMER_start(krr_TIMER* timer)
{
  timer->started = true;
  timer->paused = false;

  timer->started_ticks = SDL_GetTicks();
  timer->paused_ticks = 0;  
}

void krr_TIMER_stop(krr_TIMER* timer)
{
  // stop the timer
  timer->started = false;

  // unpause the timer
  timer->paused = false;

  // clear tick variables
  timer->started_ticks = 0;
  timer->paused_ticks = 0;
}

void krr_TIMER_pause(krr_TIMER* timer)
{
  // if the timer is running and isn't already paused
  if (timer->started && !timer->paused)
  {
    // pause the timer
    timer->paused = true;

    // calculate the paused ticks
    timer->paused_ticks = SDL_GetTicks() - timer->started_ticks;
    timer->started_ticks = 0;
  }
}

void krr_TIMER_resume(krr_TIMER* timer)
{
  // the the timer is paused and running
  if (timer->started && timer->paused)
  {
    // resume the timer (unpause the timer)
    timer->paused = false;

    // reset the starting ticks
    timer->started_ticks = SDL_GetTicks() - timer->paused_ticks;

    // reset the paused ticks
    timer->paused_ticks = 0;
  }
}

Uint32 krr_TIMER_getTicks(krr_TIMER* timer)
{
  if (timer->started)
  {
    if (timer->paused)
    {
      return timer->paused_ticks;
    }
    else
    {
      return SDL_GetTicks() - timer->started_ticks;
    }
  }

  // otherwise return 0 as ticks
  return 0;
}

void krr_TIMER_free(krr_TIMER* timer)
{
  free(timer);
  timer = NULL;
}
