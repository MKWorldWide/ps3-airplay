#include "pcm_ring.h"
#include "audio_timing.h"
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
static raop_pcm_ring ring;
/* Input already little endian. Exposed for independent Python reference tests. */
int probe(const unsigned char *in, unsigned frames, float *out, unsigned outputs,
 unsigned chunk, unsigned long long *read)
{
 unsigned cursor=882, done=0;
 assert(frames>882 && chunk>0 && chunk<=480);
 raop_pcm_ring_init(&ring,2);
 assert(raop_pcm_ring_push_le16(&ring,in,882*4)==882);
 while(done<outputs){
  unsigned n=outputs-done<chunk?outputs-done:chunk;
  unsigned long long before=ring.frames_read;
  raop_pcm_ring_render_float(&ring,44100,48000,out+done*2,n);
  unsigned used=(unsigned)(ring.frames_read-before);
  assert(cursor+used<=frames);
  assert(raop_pcm_ring_push_le16(&ring,in+cursor*4,used*4)==used || used==0);
  cursor+=used;done+=n;
 }
 *read=ring.frames_read;
 int ok=ring.output_underflows==0 && ring.frames_dropped==0;
 raop_pcm_ring_clear(&ring);
 assert(ring.count==0 && ring.phase==0 && !ring.have_current);
 return ok;
}
#ifdef PROBE_TEST
int main(void){
 raop_audio_timing t={0};
 raop_audio_timing_observe(&t,1000,20,7,8);
 raop_audio_timing_observe(&t,6333,40,0,8);
 raop_audio_timing_observe(&t,11666,15,0,8);
 raop_audio_timing_observe(&t,27665,30,3,8);
 assert(t.max_gap_us==15999 && t.max_render_us==40 && t.repeats==1 && t.skips==2);
 raop_audio_timing_reset_interval(&t);assert(t.blocks==0 && t.have_previous);
 raop_audio_timing_observe(&t,32998,12,4,8);assert(t.max_gap_us==5333 && !t.skips);
 static unsigned char input[45100*4];
 static float a[48000*2],b[48000*2];
 unsigned long long r1,r2;
 for(unsigned i=0;i<sizeof(input);i++)input[i]=(unsigned char)(i*7919U);
 assert(probe(input,45100,a,48000,1,&r1));
 assert(probe(input,45100,b,48000,480,&r2));
 assert(r1==44102 && r2==r1 && memcmp(a,b,sizeof(a))==0);
 assert(probe(input,45100,b,48000,256,&r2));
 assert(memcmp(a,b,sizeof(a))==0);
 puts("SRC chunk continuity/restart and exact 160:147 conservation PASS");
 puts("audio service aggregation: rollover, repeats, skips, reset PASS");return 0;
}
#endif
