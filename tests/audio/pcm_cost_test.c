#define _POSIX_C_SOURCE 200809L
#include "pcm_ring.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static raop_pcm_ring ring;
static unsigned long long costs[10000];
static unsigned long long ns(void){struct timespec t;assert(!clock_gettime(CLOCK_MONOTONIC,&t));return (unsigned long long)t.tv_sec*1000000000ULL+t.tv_nsec;}
static int cmp(const void*a,const void*b){unsigned long long x=*(const unsigned long long*)a,y=*(const unsigned long long*)b;return x<y?-1:x>y;}
int main(void){
 unsigned char input[882*4];float out[256*2];unsigned long long total=0;
 for(unsigned i=0;i<sizeof(input);++i)input[i]=(unsigned char)(i*7919U);
 raop_pcm_ring_init(&ring,2);assert(raop_pcm_ring_push_le16(&ring,input,sizeof(input))==882);
 for(unsigned i=0;i<10000;i++){
  unsigned long long before=ring.frames_read,t=ns();
  assert(raop_pcm_ring_render_float(&ring,44100,48000,out,256)==256);
  costs[i]=ns()-t;total+=costs[i];
  unsigned consumed=(unsigned)(ring.frames_read-before);
  assert(raop_pcm_ring_push_le16(&ring,input,consumed*4)==consumed);
 }
 assert(!ring.output_underflows && !ring.frames_dropped);
 qsort(costs,10000,sizeof(costs[0]),cmp);
 printf("stereo 256-frame SRC ns: mean=%llu p50=%llu p99=%llu max=%llu (nominal block=5333333ns)\n",total/10000,costs[5000],costs[9900],costs[9999]);return 0;
}
