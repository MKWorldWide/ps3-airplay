#ifndef RESIDENT_HOST_MOCK_SYS_THREAD_H
#define RESIDENT_HOST_MOCK_SYS_THREAD_H

#include <pthread.h>
#include <stdint.h>
#include <stddef.h>

typedef pthread_t sys_ppu_thread_t;

#define THREAD_JOINABLE 1

int sysThreadCreate(sys_ppu_thread_t *thread, void (*entry)(void *), void *arg,
	int priority, size_t stack_size, uint64_t flags, const char *name);
int sysThreadJoin(sys_ppu_thread_t thread, uint64_t *result);
int sysThreadYield(void);
void sysThreadExit(uint64_t result);

#endif
