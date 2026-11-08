/* Audit reproductions exercise unchanged production queues, not proposed fixes. */
#include "raop_rtp.h"
#include "pcm_ring.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static raop_rtp_queue q;
static raop_pcm_ring pcm;
typedef struct {unsigned seq, at;} arrival;
static arrival schedule[10000];
static int compare(const void *a,const void *b){const arrival *x=a,*y=b;return x->at<y->at?-1:x->at>y->at;}

static void put(unsigned seq, int recovered) {
 unsigned char p[13]={0x80,96,0,0,0,0,0,0,0,0,0,0,1};
 p[2]=(unsigned char)(seq>>8);p[3]=(unsigned char)seq;
 assert(recovered?raop_rtp_queue_recover(&q,p,sizeof(p)):raop_rtp_queue_push(&q,p,sizeof(p)));
}
int main(void) {
 raop_rtp_frame f; unsigned i; unsigned char input[444*4]={0};float out[480*2];
 for(i=0;i<131072;i++){put(i,0);assert(raop_rtp_queue_pop(&q,&f,i)==1);assert(f.packet.sequence==(unsigned short)i);}
 assert(q.stats.missing_packets==0); puts("in-order + two sequence wraps PASS");
 memset(&q,0,sizeof(q));put(10,0);put(12,0);put(11,0);
 for(i=10;i<=12;i++){assert(raop_rtp_queue_pop(&q,&f,0)==1);assert(f.packet.sequence==i);}
 assert(q.stats.missing_packets==1 && q.stats.concealed_packets==0);puts("reorder increments missing despite no loss: CONFIRMED");
 for(i=0;i<2;i++){
 memset(&q,0,sizeof(q));put(10,0);assert(raop_rtp_queue_pop(&q,&f,0)==1);put(12,0);
 assert(!raop_rtp_queue_pop(&q,&f,100));assert(!raop_rtp_queue_pop(&q,&f,179));
 put(11,(int)i);assert(raop_rtp_queue_pop(&q,&f,179)==1);assert(f.packet.sequence==11);
 assert(raop_rtp_queue_pop(&q,&f,180)==1);assert(q.stats.concealed_packets==0);
 }puts("delayed original/retransmission at 79ms PASS");
 memset(&q,0,sizeof(q));put(10,0);assert(raop_rtp_queue_pop(&q,&f,0)==1);put(31,0);
 assert(!raop_rtp_queue_pop(&q,&f,100));
 for(i=11;i<31;i++){assert(raop_rtp_queue_pop(&q,&f,180)==2);assert(f.packet.sequence==i);}
 assert(q.stats.concealed_packets==20);assert(raop_rtp_queue_pop(&q,&f,180)==1);
 puts("20-frame burst concealed after one 80ms wait: CONFIRMED");
 memset(&q,0,sizeof(q));put(10,0);assert(raop_rtp_queue_pop(&q,&f,0)==1);put(12,0);
 assert(!raop_rtp_queue_pop(&q,&f,100));assert(raop_rtp_queue_pop(&q,&f,180)==2);
 {unsigned char p[13]={0x80,96,0,11,0,0,0,0,0,0,0,0,1};assert(!raop_rtp_queue_recover(&q,p,sizeof(p)));}
 assert(q.stats.late_packets==1);puts("resend after concealment rejected PASS");
 raop_rtp_queue_flush(&q);put(40000,0);assert(raop_rtp_queue_pop(&q,&f,0)==1);puts("flush sequence epoch reset PASS");
 raop_pcm_ring_init(&pcm,2);assert(raop_pcm_ring_push_le16(&pcm,input,8)==2);
 for(i=0;i<60000;i++) {assert(raop_pcm_ring_push_le16(&pcm,input,441*4)==441);assert(raop_pcm_ring_render_float(&pcm,44100,48000,out,480)==480);}
 assert(pcm.output_underflows==0);assert(pcm.frames_read==26460002);assert(pcm.phase==0);
 printf("600s SRC: output=28800000 input=%llu lookahead=2 underflows=0 phase=0 PASS\n",(unsigned long long)pcm.frames_read);
 memset(&q,0,sizeof(q));
 {unsigned cursor=0, played=0, tick;
 for(i=0;i<10000;i++){schedule[i].seq=i;schedule[i].at=i*8U+(i*7919U%601U);}
 qsort(schedule,10000,sizeof(schedule[0]),compare);
 for(tick=0;played<10000 && tick<82000;tick++){
 while(cursor<10000 && schedule[cursor].at<=tick){put(schedule[cursor].seq,0);cursor++;}
 if(tick>=1024 && (tick-1024)%8==0){assert(raop_rtp_queue_pop(&q,&f,tick)==1);assert(f.packet.sequence==played);played++;}
 }
 assert(played==10000 && q.stats.concealed_packets==0 && q.stats.queue_drops==0);
 printf("10000 packets, 0..600ms jitter, 1024ms playout lead: no concealment; high-water missing=%llu PASS\n",(unsigned long long)q.stats.missing_packets);
 }
 return 0;
}
