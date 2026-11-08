"""Benchmark the actual framebuffer copy with synthetic RGB; host time only."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[2]
s=(root/'prototype/standalone-receiver/source/diagnostic_screen.c').read_text()
a=s.index('static void copy_image(');b=s.index('static int wait_rsx_idle',a)
head='''#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#define IMAGE_WIDTH 1280U
#define IMAGE_HEIGHT 720U
static uint64_t ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000000ULL+t.tv_nsec;}
'''
tail='''int main(void){
 volatile uint16_t width=1920,height=1080;
 uint32_t *dst=malloc(1920*1080*4);uint8_t *rgb=malloc(1280*720*3);
 assert(dst && rgb);for(unsigned i=0;i<1280*720*3;i++)rgb[i]=(uint8_t)(i*79);
 uint64_t sum=0,max=0;
 for(int i=0;i<20;i++){uint64_t t=ns();copy_image(dst,1920*4,width,height,rgb);t=ns()-t;sum+=t;if(t>max)max=t;}
 assert(dst[0]==((uint32_t)rgb[0]<<16 | (uint32_t)rgb[1]<<8 | rgb[2]));
 printf("1920x1080 framebuffer copy ns: mean=%llu max=%llu (host only)\\n",(unsigned long long)(sum/20),(unsigned long long)max);
 free(dst);free(rgb);return 0;}
'''
out=root/'build/forensic';out.mkdir(parents=True,exist_ok=True)
(out/'copy-cost.c').write_text(head+s[a:b]+tail)
subprocess.run(['cc','-O2','-Wall','-Wextra','-Werror',str(out/'copy-cost.c'),'-o',str(out/'copy-cost')],check=True)
subprocess.run([str(out/'copy-cost')],check=True)
