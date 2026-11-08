/* Include translation unit to inspect a private recovery policy without edits. */
#include "../../src/protocol/raop_receiver.c"
#include <assert.h>
int main(void) {
 static raop_receiver r;
 r.session.peer_control_port=1;
 queue_missing_audio(&r,100,17);
 assert(r.pending_resend_count==0);
 assert(r.resend_pending_drops==0);
 queue_missing_audio(&r,100,16);
 assert(r.pending_resend_count==1);
 puts("17-packet gap: silently unrequested; 16-packet gap: queued CONFIRMED");
 return 0;
}
