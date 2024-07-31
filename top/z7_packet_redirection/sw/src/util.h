#include <xstatus.h>
#include "xil_io.h"
#include "xscugic.h"
#include "xemacps.h"
#include "xil_cache.h"
#include "xparameters.h"


#define EMACPS_LOOPBACK_SPEED_1G 1000	/* 1000Mbps */

char EmacPsMAC[] = { 0x00, 0x0a, 0x35, 0x01, 0x02, 0x03 };  //MAC address to configure

typedef char EthernetFrame[XEMACPS_MAX_VLAN_FRAME_SIZE] __attribute__ ((aligned(64)));

void EmacpsDelay(u32 delay);

// void EmacPsUtilSetupUart(void);
// void EmacPsUtilFrameHdrFormatMAC(EthernetFrame *FramePtr, char *DestAddr);
// void EmacPsUtilFrameHdrFormatType(EthernetFrame *FramePtr, u16 FrameType);
// void EmacPsUtilFrameSetPayloadData(EthernetFrame *FramePtr, u32 PayloadSize);
// LONG EmacPsUtilFrameVerify(EthernetFrame *CheckFrame, EthernetFrame *ActualFrame);
// void EmacPsUtilFrameMemClear(EthernetFrame *FramePtr);
// void EmacPsUtilstrncpy(char *Destination, const char *Source, u32 n);

LONG EmacPsUtilEnterLoopback(XEmacPs *EmacPsInstancePtr, u32 Speed);
u32 XEmacPsDetectPHY(XEmacPs *EmacPsInstancePtr);
LONG EmacPsUtilMarvellPhyLoopback(XEmacPs *EmacPsInstancePtr, u32 Speed, u32 PhyAddr);
LONG EmacPsUtilTiPhyLoopback(XEmacPs *EmacPsInstancePtr, u32 Speed, u32 PhyAddr);
void EmacPsUtilErrorTrap(const char *Message);
