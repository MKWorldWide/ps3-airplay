#ifndef RAOP_MBEDTLS_PS3_USER_CONFIG_H
#define RAOP_MBEDTLS_PS3_USER_CONFIG_H
/* The PS3 PPU has 64-bit registers but 32-bit pointers. Mbed TLS's PPC64
 * bn_mul.h assembly loads/stores pointers with ld/std (8 bytes), corrupting
 * pointer slots in this ABI. Use the portable arithmetic on this target. */
#if defined(__powerpc64__) && __SIZEOF_POINTER__ == 4
#undef MBEDTLS_HAVE_ASM
#endif
#endif
