/* Include translation unit to inspect a private recovery policy without edits. */
#include "../../src/protocol/raop_receiver.c"
#include <assert.h>
int main(void) {
 static raop_receiver r;
 r.session.peer_control_port=1;
 queue_missing_audio(&r,100,17);
 assert(r.pending_resend_count==2);
 assert(r.pending_resend[0].packet_count==16);
 assert(r.pending_resend[1].first_sequence==116 && r.pending_resend[1].packet_count==1);
 assert(r.resend_pending_drops==0);
 queue_missing_audio(&r,100,16);
 assert(r.pending_resend_count==3);
 {
  int sink=socket(AF_INET,SOCK_DGRAM,0);struct sockaddr_in addr={0};socklen_t len=sizeof(addr);
  unsigned char datagram[8];struct timeval timeout={0,50000};
  assert(sink>=0);addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  assert(bind(sink,(struct sockaddr *)&addr,sizeof(addr))==0);
  assert(getsockname(sink,(struct sockaddr *)&addr,&len)==0);
  assert(setsockopt(sink,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout))==0);
  memset(&r,0,sizeof(r));r.control_fd=socket(AF_INET,SOCK_DGRAM,0);assert(r.control_fd>=0);
  r.peer_address=addr;r.session.state=RAOP_SESSION_RECORDING;r.session.peer_control_port=ntohs(addr.sin_port);
  queue_missing_audio(&r,100,2);send_pending_audio_resend(&r,1000);
  assert(recv(sink,datagram,sizeof(datagram),0)==8);assert(r.resend_requests==1);
  send_pending_audio_resend(&r,1100);assert(recv(sink,datagram,sizeof(datagram),0)==8);
  send_pending_audio_resend(&r,1200);assert(recv(sink,datagram,sizeof(datagram),0)==8);
  send_pending_audio_resend(&r,1300);assert(recv(sink,datagram,sizeof(datagram),0)<0);
  assert(r.resend_retries==2 && r.outstanding_resend[0].attempts==3);
  close(sink);close(r.control_fd);
 }
 puts("bounded resend retries (initial + 2, then stop): PASS");
 puts("17-packet gap split into 16+1; 16-packet gap queued: PASS");
 return 0;
}
