#ifndef RAOP_AUDIO_TIMING_H
#define RAOP_AUDIO_TIMING_H
#include <stdint.h>
/* Fixed storage, no clocks or I/O here. Service gaps are observations, not xruns.
 * Modular read-index jumps cannot detect a whole buffer-cycle lapse. */
typedef struct {
 uint64_t previous_us, max_gap_us, max_render_us, blocks, repeats, skips;
 uint32_t previous_index;
 int have_previous;
} raop_audio_timing;
static inline void raop_audio_timing_observe(raop_audio_timing *t,
 uint64_t now, uint64_t render_us, uint32_t index, uint32_t blocks)
{
 if(t->have_previous && now>=t->previous_us) {
  uint64_t gap=now-t->previous_us;
  uint32_t step=(index+blocks-t->previous_index)%blocks;
  if(gap>t->max_gap_us)t->max_gap_us=gap;
  if(step==0)++t->repeats;
  else if(step>1)t->skips+=step-1;
 }
 if(render_us>t->max_render_us)t->max_render_us=render_us;
 t->previous_us=now;t->previous_index=index;t->have_previous=1;++t->blocks;
}
static inline void raop_audio_timing_reset_interval(raop_audio_timing *t)
{
 t->max_gap_us=t->max_render_us=t->blocks=t->repeats=t->skips=0;
}
#endif
