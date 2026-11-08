"""Exercise production presenter with deterministic RSX/clock mocks."""
from pathlib import Path
import subprocess
root = Path(__file__).resolve().parents[2]
src = (root/'prototype/standalone-receiver/source/diagnostic_screen.c').read_text()
body = src[src.index('void diagnostic_screen_invalidate'):src.index('int diagnostic_screen_close')]
header = r'''#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "diagnostic_screen.h"
typedef int gcmContextData;
#define SCREEN_REFRESH_INTERVAL_US 16667U
static uint64_t clock_us=1000;
static int busy, flips, copies;
static uint64_t sysGetSystemTime(void){return clock_us;}
static int gcmGetFlipStatus(void){return busy;}
static void gcmResetFlipStatus(void){}
static int gcmSetFlip(gcmContextData *c,uint32_t n){(void)c;(void)n;++flips;return 0;}
static void rsxFlushBuffer(gcmContextData *c){(void)c;}
static void gcmSetWaitFlip(gcmContextData *c){(void)c;}
static void copy_image(uint32_t *p,uint32_t pitch,uint16_t w,uint16_t h,const uint8_t *rgb){(void)p;(void)pitch;(void)w;(void)h;(void)rgb;++copies;}
'''
tail = r'''
int main(void){
 diagnostic_screen s={0};int i;
 s.initialized=1;diagnostic_screen_invalidate(&s);
 for(i=0;i<130;i++){clock_us+=16667;assert(diagnostic_screen_present(&s,0)>=0);}
 assert(flips>=120 && s.resume_until_us==0 && !s.flip_pending && s.redraw_mask==0);
 {int n=flips;for(i=0;i<100;i++){clock_us+=16667;assert(diagnostic_screen_present(&s,0)==0);}assert(n==flips);}
 assert(copies>=2);
 /* A menu flip may remain pending; resume gets a fresh bounded timeout. */
 clock_us+=16667;assert(diagnostic_screen_present(&s,1)>0);busy=1;
 clock_us+=1000000;assert(diagnostic_screen_present(&s,1)==0);
 diagnostic_screen_invalidate(&s);clock_us+=499999;assert(diagnostic_screen_present(&s,0)==0);
 clock_us+=2;assert(diagnostic_screen_present(&s,0)==DIAGNOSTIC_SCREEN_TIMEOUT);
 puts("production display resume: paced settling, static completion, bounded stalled flip PASS");return 0;
}
'''
out=root/'build/forensic';out.mkdir(parents=True,exist_ok=True)
(out/'display.c').write_text(header+body+tail)
subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(root/'prototype/standalone-receiver/source'),str(out/'display.c'),'-o',str(out/'display')],check=True)
subprocess.run([str(out/'display')],check=True)
