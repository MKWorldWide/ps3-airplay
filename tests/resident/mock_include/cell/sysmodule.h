#ifndef RESIDENT_HOST_MOCK_CELL_SYSMODULE_H
#define RESIDENT_HOST_MOCK_CELL_SYSMODULE_H

#define CELL_SYSMODULE_NET 1
#define CELL_SYSMODULE_NETCTL 2
#define CELL_SYSMODULE_AUDIO 3

int cellSysmoduleLoadModule(int module);
int cellSysmoduleUnloadModule(int module);

#endif
