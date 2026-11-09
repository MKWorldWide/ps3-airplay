#include <stddef.h>
#include <stdint.h>
#include <sys/prx.h>
#include <sys/moduleexport.h>

SYS_MODULE_INFO(resident_loader_reference, 0, 1, 0);
SYS_MODULE_START(resident_loader_reference_start);
SYS_MODULE_STOP(resident_loader_reference_stop);

static volatile uint32_t start_count;
static volatile uint32_t stop_count;

int resident_loader_reference_identity(void)
{
	return 0x52505258;
}

int resident_loader_reference_start(size_t args, void *argp)
{
	(void)args;
	(void)argp;
	++start_count;
	return SYS_PRX_RESIDENT;
}

int resident_loader_reference_stop(size_t args, void *argp)
{
	(void)args;
	(void)argp;
	++stop_count;
	return SYS_PRX_STOP_OK;
}
