#ifndef RESIDENT_HOST_MOCK_CELL_LIBNETCTL_H
#define RESIDENT_HOST_MOCK_CELL_LIBNETCTL_H

typedef union CellNetCtlInfo {
	struct { unsigned char data[6]; } ether_addr;
} CellNetCtlInfo;

#define CELL_NET_CTL_INFO_ETHER_ADDR 1

int cellNetCtlInit(void);
void cellNetCtlTerm(void);
int cellNetCtlGetInfo(int code, union CellNetCtlInfo *info);

#endif
