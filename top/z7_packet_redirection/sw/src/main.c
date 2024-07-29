#include <stdio.h>
#include <xstatus.h>
#include "xil_io.h"
#include "xscugic.h"
#include "xemacps.h"
#include "xparameters.h"

#define ETH0_INT_ID 54U
#define ETH0_WAKE_INT_ID 55U
static XScuGic IntcInstance; // Interrupt Controller Instance
static XEmacPs EmacInstance; // Ethernet MAC Controller

/************************************************************************
 * Initializes the GIC
 *
 * @param IntcInstance - a pointer to a XScuGic type
 * @param CfgPtr - a pointer to a XScuGic_Config type
 * @param BaseAddr - the effective/baseadddr of the GIC to initialize
 *
 * @return
 *  - XST_SUCCESS initialization passed
 *  - XST_FAILED initialization failed
 *************************************************************************/

 static inline s32 init_gic(XScuGic* IntcInstance, XScuGic_Config* CfgPtr, u32 BaseAddr) {
    u32 status;
     // Initialize the GIC
    CfgPtr = XScuGic_LookupConfig(BaseAddr);
    status =XScuGic_CfgInitialize(IntcInstance, CfgPtr, BaseAddr);

    if(status != XST_SUCCESS) {
        xil_printf("Error in configuring/initializing the GIC\n\r");
        return XST_FAILURE;
    }
    if(XScuGic_IsInitialized(BaseAddr)!= 1U) {
        xil_printf("XScuGic_IsInitialized Failed\n\r");
        return XST_FAILURE;
    }
    if(XScuGic_SelfTest(IntcInstance)!= XST_SUCCESS) {
        xil_printf("XScuGic_SelfTest Failed\n\r");
        return XST_FAILURE;
    }

    xil_printf("SCUGIC.name=%s \n\r SCUGIC.DistBAddr=0x%x \n\r SCUGIC.CpuBAddr=0x%x\n\r", 
    CfgPtr->Name, CfgPtr->DistBaseAddress, CfgPtr->CpuBaseAddress);

    return XST_SUCCESS;

 }

 /************************************************************************
 * Initializes the Ethernet MAC PS Controller
 *
 * @param EmacInstance - a pointer to a XEmacPs type
 * @param CfgPtr - a pointer to a XEmacPs_Config type
 * @param BaseAddr - the effective/baseadddr of the GIC to initialize
 *
 * @return
 *  - XST_SUCCESS initialization passed
 *  - XST_FAILED initialization failed
 *************************************************************************/

 static inline s32 init_emac(XEmacPs* EmacInstance, XEmacPs_Config* CfgPtr, u32 BaseAddr) {
    u32 status, GemVersion;
     // Initialize the GIC
    CfgPtr = XEmacPs_LookupConfig(BaseAddr);
    status =XEmacPs_CfgInitialize(EmacInstance, CfgPtr, CfgPtr->BaseAddress);
    if(status != XST_SUCCESS) {
        xil_printf("Error in configuring/initializing the EMAC\n\r");
        return XST_FAILURE;
    }
    GemVersion = ((Xil_In32(CfgPtr->BaseAddress + 0xFC)) >> 16) & 0xFFF; // geting GEM version to see if we need additional clock setup
    xil_printf("EMAC.name=%s \n\r Emac.BAddr=0x%x\n\r Emac.Intrid=0x%x\n\r Emac.PhyType=%s \n\r Emac.GemVersion=%0d",
    CfgPtr->Name, CfgPtr->BaseAddress, CfgPtr->IntrId, CfgPtr->PhyType,GemVersion);
    return XST_SUCCESS;

 }

int main () {
    XScuGic_Config Gic_CfgPtr;
    XEmacPs_Config Emac_CfgPtr;

    u32 status;

    print("Hello World\n\r");

    print("Initializing GIC_0\n\r");
    status = init_gic(&IntcInstance, &Gic_CfgPtr, XPAR_XSCUGIC_0_BASEADDR);
    if(status != XST_SUCCESS) {
        print("Initialization of GIC failed\n\r");
    }

    print("Initializing the EMAC_0\n\r");
    status = init_emac(&EmacInstance, &Emac_CfgPtr, XPAR_XEMACPS_0_BASEADDR);
    if(status != XST_SUCCESS) {
        print("Initialization of EMAC failed\n\r");
    }

    print("Enabling the GIC interrupts\n\r");
    XScuGic_Enable(&IntcInstance, ETH0_INT_ID);
    XScuGic_Enable(&IntcInstance, ETH0_WAKE_INT_ID);

    print("Leaving Main");

return 0;
}