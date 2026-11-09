#include <stddef.h>
#include <sys/prx.h>

int module_start(size_t args, void *argp)
{
	(void)args;
	(void)argp;
	return SYS_PRX_RESIDENT;
}

int module_stop(size_t args, void *argp)
{
	(void)args;
	(void)argp;
	return SYS_PRX_STOP_OK;
}
