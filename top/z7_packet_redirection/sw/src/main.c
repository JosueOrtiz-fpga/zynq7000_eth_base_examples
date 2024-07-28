#include <stdio.h>
#include <xstatus.h>
#include "xil_io.h"
#include "xscugic.h"
#include "xparameters.h"

#define ETH0_INT_ID 54U
#define ETH0_WAKE_INT_ID 55U
static XScuGic IntcInstance; // Interrupt Controller Instance

/*****************************************
 * Initializes the GIC
 *
 * @param IntcInstance - a pointer to a XScuGic type
 * @param CfgPtr - a pointer to a XScuGic_Config type
 * @param BaseAddr - the effective/baseadddr of the GIC to initialize
 *
 * @return
 *  - XST_SUCCESS initialization passed
 *  - XST_FAILED initialization failed
 ******************************************/

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

int main () {
    XScuGic_Config CfgPtr;
    u32 status;

    print("Hello World\n\r");

    print("Initializing GIC_0\n\r");
    status = init_gic(&IntcInstance, &CfgPtr, XPAR_XSCUGIC_0_BASEADDR);
    if(status != XST_SUCCESS) {
        print("Initialization of GIC failed\n\r");
    }

    print("Enabling the GIC interrupts\n\r");
    XScuGic_Enable(&IntcInstance, ETH0_INT_ID);
    XScuGic_Enable(&IntcInstance, ETH0_WAKE_INT_ID);

    print("Leaving Main");

return 0;
}