#include "raop_sdp.h"

#include <string.h>

static int starts_with(const char *line, size_t length, const char *prefix)
{
	size_t prefix_length = strlen(prefix);
	return length >= prefix_length && memcmp(line, prefix, prefix_length) == 0;
}

static int decimal(const char *text, size_t length, uint32_t *value)
{
	uint32_t result = 0;
	size_t i;
	if (length == 0)
		return 0;
	for (i = 0; i < length; ++i) {
		uint32_t digit;
		if (text[i] < '0' || text[i] > '9')
			return 0;
		digit = (uint32_t)(text[i] - '0');
		if (result > (UINT32_MAX - digit) / 10U)
			return 0;
		result = result * 10U + digit;
	}
	*value = result;
	return 1;
}

static int parse_fmtp(const char *text, size_t length, uint32_t values[11])
{
	size_t pos = 0;
	unsigned int i;
	for (i = 0; i < 11; ++i) {
		size_t start = pos;
		while (pos < length && text[pos] >= '0' && text[pos] <= '9')
			++pos;
		if (start == pos || !decimal(text + start, pos - start, &values[i]))
			return 0;
		if (i < 10) {
			if (pos >= length || text[pos] != ' ')
				return 0;
			while (pos < length && text[pos] == ' ')
				++pos;
		}
	}
	return pos == length;
}

static int copy_base64(char *out, size_t capacity, size_t *out_length,
	const char *text, size_t length)
{
	size_t i;
	if (length == 0 || length >= capacity)
		return 0;
	for (i = 0; i < length; ++i) {
		char c = text[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') || c == '+' || c == '/' || c == '='))
			return 0;
	}
	memcpy(out, text, length);
	out[length] = '\0';
	*out_length = length;
	return 1;
}

int raop_sdp_parse(const char *data, size_t length, raop_sdp_offer *offer)
{
	raop_sdp_offer parsed;
	size_t offset = 0;
	if (data == NULL || offer == NULL || length == 0 || length > 4096)
		return 0;
	memset(&parsed, 0, sizeof(parsed));
	while (offset < length) {
		size_t start = offset;
		size_t line_length;
		while (offset < length && data[offset] != '\r' && data[offset] != '\n') {
			if (data[offset] == '\0' ||
				((unsigned char)data[offset] < 0x20 && data[offset] != '\t'))
				return 0;
			++offset;
		}
		line_length = offset - start;
		if (offset < length && data[offset] == '\r') {
			++offset;
			if (offset >= length || data[offset] != '\n')
				return 0;
			++offset;
		} else if (offset < length) {
			++offset;
		}
		if (line_length == 0)
			continue;
		if (line_length == 3 && memcmp(data + start, "v=0", 3) == 0) {
			if (parsed.has_version) return 0;
			parsed.has_version = 1;
		} else if (starts_with(data + start, line_length, "m=audio ")) {
			if (parsed.has_audio) return 0;
			parsed.has_audio = 1;
		} else if (starts_with(data + start, line_length,
			"a=rtpmap:96 AppleLossless") &&
			(line_length == sizeof("a=rtpmap:96 AppleLossless") - 1 ||
			data[start + sizeof("a=rtpmap:96 AppleLossless") - 1] == '/')) {
			if (parsed.has_alac) return 0;
			parsed.has_alac = 1;
		} else if (starts_with(data + start, line_length, "a=fmtp:96 ")) {
			if (parsed.has_fmtp || !parse_fmtp(data + start + 10,
				line_length - 10, parsed.fmtp))
				return 0;
			parsed.has_fmtp = 1;
		} else if (starts_with(data + start, line_length, "a=rsaaeskey:")) {
			if (parsed.has_rsa_aes_key || !copy_base64(parsed.rsa_aes_key,
				sizeof(parsed.rsa_aes_key), &parsed.rsa_aes_key_length,
				data + start + 12, line_length - 12))
				return 0;
			parsed.has_rsa_aes_key = 1;
		} else if (starts_with(data + start, line_length, "a=aesiv:")) {
			if (parsed.has_aes_iv || !copy_base64(parsed.aes_iv,
				sizeof(parsed.aes_iv), &parsed.aes_iv_length,
				data + start + 8, line_length - 8))
				return 0;
			parsed.has_aes_iv = 1;
		} else if (starts_with(data + start, line_length, "a=et:")) {
			uint32_t value;
			if (parsed.has_encryption_type || !decimal(data + start + 5,
				line_length - 5, &value) || value > UINT8_MAX)
				return 0;
			parsed.has_encryption_type = 1;
			parsed.encryption_type = (uint8_t)value;
		}
	}
	if (!parsed.has_version || !parsed.has_audio)
		return 0;
	*offer = parsed;
	return 1;
}
