#ifndef RESIDENT_HOST_MOCK_CRYPTO_RNG_H
#define RESIDENT_HOST_MOCK_CRYPTO_RNG_H

#include <stddef.h>

int raop_crypto_ps3_rng(void *context, unsigned char *output, size_t length);

#endif
