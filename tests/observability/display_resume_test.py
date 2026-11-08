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
 assert(flips>=(int)(DIAGNOSTIC_SCREEN_RESUME_US/16667U) && flips<=(int)(DIAGNOSTIC_SCREEN_RESUME_US/16667U)+4 && s.resume_until_us==0 && !s.flip_pending && s.redraw_mask==0);
 {int n=flips;for(i=0;i<100;i++){clock_us+=16667;assert(diagnostic_screen_present(&s,0)==0);}assert(n==flips);}
 assert(copies>=2);
 /* Repeated menu and rapid invalidate cycles preserve bounded restoration. */
 for(int cycle=0;cycle<8;cycle++){
  for(i=0;i<3;i++){clock_us+=16667;assert(diagnostic_screen_present(&s,1)>=0);}
  diagnostic_screen_invalidate(&s);uint64_t start=clock_us;
  do{clock_us+=16667;assert(diagnostic_screen_present(&s,0)>=0);}
  while(s.resume_until_us || s.flip_pending || s.redraw_mask);
  assert(clock_us-start<=DIAGNOSTIC_SCREEN_RESUME_US+50001U);
  if(DIAGNOSTIC_SCREEN_RESUME_US<=350000U)assert(clock_us-start<=500000U);
 }
 diagnostic_screen_invalidate(&s);clock_us+=10000;diagnostic_screen_invalidate(&s);
 for(i=0;i<(int)(DIAGNOSTIC_SCREEN_RESUME_US/16667U)+5;i++){clock_us+=16667;assert(diagnostic_screen_present(&s,0)>=0);}
 assert(!s.resume_until_us && !s.flip_pending && !s.redraw_mask);
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
for window in [250000,350000,500000,750000]:
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-DDIAGNOSTIC_SCREEN_RESUME_US='+str(window),'-I'+str(root/'prototype/standalone-receiver/source'),str(out/'display.c'),'-o',str(out/'display')],check=True)
    subprocess.run([str(out/'display')],check=True)

main_src=(root/"prototype/standalone-receiver/source/main.c").read_text()
a=main_src.index("static void on_system_event("); b=main_src.index("\nstatic int log_event(",a)
(out/"callback.c").write_text('#include <stdint.h>\n#include <assert.h>\ntypedef uint64_t u64;\n#define SYSUTIL_DRAW_BEGIN 289\n#define SYSUTIL_DRAW_END 290\n#define SYSUTIL_MENU_OPEN 305\n#define SYSUTIL_MENU_CLOSE 306\n#define SYSUTIL_EXIT_GAME 257\nstatic int system_drawing,system_menu_open,presentation_failed,resume_pending,exit_requested,screen,invalidations;\nstatic int log_event(const char*e,int v,unsigned long n){(void)e;(void)v;(void)n;return 1;}\nstatic void diagnostic_screen_invalidate(int*s){(void)s;++invalidations;}\n'+main_src[a:b]+'\nint main(void){for(int i=0;i<8;i++){\non_system_event(SYSUTIL_DRAW_BEGIN,0,0);on_system_event(SYSUTIL_MENU_OPEN,0,0);\nassert(system_drawing && system_menu_open);\npresentation_failed=1;on_system_event(SYSUTIL_MENU_CLOSE,0,0);assert(!system_menu_open && system_drawing && resume_pending && !presentation_failed);\non_system_event(SYSUTIL_DRAW_END,0,0);assert(!system_drawing && !system_menu_open);}\nassert(invalidations==16);on_system_event(SYSUTIL_EXIT_GAME,0,0);assert(exit_requested);return 0;}\n')
subprocess.run(["cc","-std=c11","-Wall","-Wextra","-Werror","-fsanitize=address,undefined",str(out/"callback.c"),"-o",str(out/"callback")],check=True)
subprocess.run([str(out/"callback")],check=True)
