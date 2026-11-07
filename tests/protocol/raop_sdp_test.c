#include "../../src/protocol/raop_sdp.h"
#include "../../src/protocol/raop_session.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	static const char sdp[] =
		"v=0\r\n"
		"o=iTunes 123 0 IN IP4 192.0.2.1\r\n"
		"s=iTunes\r\n"
		"m=audio 0 RTP/AVP 96\r\n"
		"a=rtpmap:96 AppleLossless/44100/2\r\n"
		"a=fmtp:96 352 0 16 40 10 14 2 255 0 0 44100\r\n"
		"a=rsaaeskey:AQIDBA==\r\n"
		"a=aesiv:AAECAwQFBgcICQoLDA0ODw==\r\n"
		"a=et:0\r\n";
	raop_sdp_offer offer;
	raop_session session;
	char announce[1024];
	char long_key_sdp[512];
	char long_key[345];
	char response[RAOP_SESSION_RESPONSE_MAX];
	int n;

	assert(raop_sdp_parse(sdp, sizeof(sdp) - 1, &offer));
	assert(offer.has_version && offer.has_audio && offer.has_alac);
	assert(offer.has_fmtp && offer.fmtp[0] == 352 && offer.fmtp[2] == 16);
	assert(offer.fmtp[6] == 2 && offer.fmtp[10] == 44100);
	assert(offer.has_rsa_aes_key && offer.rsa_aes_key_length == 8);
	assert(offer.has_aes_iv && offer.aes_iv_length == 24);
	assert(offer.has_encryption_type && offer.encryption_type == 0);
	memset(long_key, 'A', sizeof(long_key) - 1);
	long_key[sizeof(long_key) - 3] = '=';
	long_key[sizeof(long_key) - 2] = '=';
	long_key[sizeof(long_key) - 1] = '\0';
	n = snprintf(long_key_sdp, sizeof(long_key_sdp),
		"v=0\r\nm=audio 0 RTP/AVP 96\r\na=rsaaeskey:%s\r\n", long_key);
	assert(n > 0 && (size_t)n < sizeof(long_key_sdp));
	assert(raop_sdp_parse(long_key_sdp, (size_t)n, &offer));
	assert(offer.rsa_aes_key_length == sizeof(long_key) - 1);

	assert(!raop_sdp_parse("v=0\r\nm=video 0 RTP/AVP 96\r\n",
		sizeof("v=0\r\nm=video 0 RTP/AVP 96\r\n") - 1, &offer));
	assert(!raop_sdp_parse("v=0\r\nm=audio 0 RTP/AVP 96\r\na=fmtp:96 1 2\r\n",
		sizeof("v=0\r\nm=audio 0 RTP/AVP 96\r\na=fmtp:96 1 2\r\n") - 1,
		&offer));
	assert(!raop_sdp_parse("v=0\r\nm=audio 0 RTP/AVP 96\r\na=rsaaeskey:bad!\r\n",
		sizeof("v=0\r\nm=audio 0 RTP/AVP 96\r\na=rsaaeskey:bad!\r\n") - 1,
		&offer));

	raop_session_init(&session, 6000, 6001, 6002);
	n = snprintf(announce, sizeof(announce),
		"ANNOUNCE rtsp://ps3/stream RTSP/1.0\r\nCSeq: 1\r\n"
		"Content-Type: application/sdp\r\nContent-Length: %zu\r\n\r\n%s",
		sizeof(sdp) - 1, sdp);
	assert(n > 0 && (size_t)n < sizeof(announce));
	assert(raop_session_handle(&session, announce, (size_t)n, response,
		sizeof(response)));
	assert(session.state == RAOP_SESSION_ANNOUNCED);
	assert(session.offer.has_rsa_aes_key && session.offer.has_aes_iv);
	assert(raop_session_handle(&session,
		"TEARDOWN rtsp://ps3/stream RTSP/1.0\r\nCSeq: 2\r\n\r\n",
		sizeof("TEARDOWN rtsp://ps3/stream RTSP/1.0\r\nCSeq: 2\r\n\r\n") - 1,
		response, sizeof(response)));
	assert(session.offer.rsa_aes_key_length == 0);

	puts("RAOP bounded SDP codec/key parsing: PASS");
	return 0;
}
