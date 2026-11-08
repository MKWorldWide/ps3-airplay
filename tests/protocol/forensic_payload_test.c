/* Synthetic FFmpeg fixture: full-entropy 352-sample stereo ALAC. */
#include "raop_alac.h"
#include "raop_rtp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static size_t read_file(const char *name,unsigned char *data,size_t cap){FILE *f=fopen(name,"rb");size_t n;assert(f);n=fread(data,1,cap,f);assert(feof(f));fclose(f);return n;}
int main(void){
 unsigned char packet[2048]={0x80,96}, expected[2048], output[16384];
 size_t n=read_file("tests/protocol/fixtures/alac-noise-352.packet",packet+12,sizeof(packet)-12);
 size_t bytes=read_file("tests/protocol/fixtures/alac-noise-352.pcm",expected,sizeof(expected)),decoded=0;
 uint32_t samples=0;raop_rtp_packet parsed;raop_sdp_offer offer;raop_alac_decoder *d;int result;
 memset(&offer,0,sizeof(offer));offer.has_fmtp=1;offer.has_alac=1;
 {uint32_t fmtp[11]={352,0,16,40,10,14,2,0,0,0,44100};memcpy(offer.fmtp,fmtp,sizeof(fmtp));}
 assert(n==1416 && bytes==1408);
 result=raop_rtp_parse(packet,n+12,&parsed);assert(result==(RAOP_RTP_PAYLOAD_MAX>=1416));
 printf("valid 1416-byte ALAC RTP parse=%d (cap=%d)\n",result,RAOP_RTP_PAYLOAD_MAX);
 d=raop_alac_create(&offer);assert(d);result=raop_alac_decode(d,packet+12,n,output,sizeof(output),&decoded,&samples);
 assert(result==(RAOP_ALAC_INPUT_MAX>=1416));
 if(result){assert(samples==352 && decoded==bytes);assert(!memcmp(output,expected,bytes));}
 printf("ALAC decode=%d (cap=%d), exact_reference_pcm=%d\n",result,RAOP_ALAC_INPUT_MAX,result);
 raop_alac_destroy(d);return 0;
}
