#define _POSIX_C_SOURCE 200809L
#include "receiver_log.h"
#include <time.h>
#include <stdlib.h>
#include <assert.h>
static unsigned long long now_ns(void){struct timespec t;assert(!clock_gettime(CLOCK_MONOTONIC,&t));return (unsigned long long)t.tv_sec*1000000000ULL+t.tv_nsec;}
static int cmp(const void*a,const void*b){unsigned long long x=*(const unsigned long long*)a,y=*(const unsigned long long*)b;return x<y?-1:x>y;}
int main(void){
 receiver_log_file log={0};unsigned long long costs[10000],sum=0;
 log.stream=tmpfile();assert(log.stream);
 for(int i=0;i<10000;i++){unsigned long long t=now_ns();assert(receiver_log_file_event(&log,"2000-01-01T00:00:00Z","BENCH_EVENT",i,0));costs[i]=now_ns()-t;sum+=costs[i];}
 qsort(costs,10000,sizeof(costs[0]),cmp);
 printf("log format/write/fflush ns: mean=%llu p50=%llu p99=%llu max=%llu\n",sum/10000,costs[5000],costs[9900],costs[9999]);
 receiver_log_file_close(&log);return 0;
}
