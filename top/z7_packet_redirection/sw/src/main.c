#include <stdio.h>
#include <xstatus.h>
#include "xil_io.h"
#include "xscugic.h"
#include "xemacps.h"
#include "xil_mmu.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xparameters.h"

#define ETH0_INT_ID 54U
#define ETH0_WAKE_INT_ID 55U

#define RXBD_CNT       32	// Number of RxBDs to use
#define TXBD_CNT       32	// Number of TxBDs to use

static XScuGic IntcInstance; // Interrupt Controller Instance
static XEmacPs EmacInstance; // Ethernet MAC Controller

char EmacPsMAC[] = { 0x00, 0x0a, 0x35, 0x01, 0x02, 0x03 };  //MAC address to configure
u32 RxCounter, TxCounter, DeviceErrors;                     // Counters to track num of

typedef char EthernetFrame[XEMACPS_MAX_VLAN_FRAME_SIZE] __attribute__ ((aligned(64)));
EthernetFrame TxFrame;		// Transmit buffer
EthernetFrame RxFrame;		// Receive buffer


u8 *RxBdSpacePtr; // pointer to Rx BD Ring
u8 *TxBdSpacePtr; // pointer to Tx BD Ring
u8 bd_space[0x100000]; // Memory allocation for BD rings;
                       // 1MB is the smalles allotment MMU attributes
                       // can be applied to



static void XEmacPsRecvHandler(void *Callback); // MAC rx callback protoype
static void XEmacPsSendHandler(void *Callback); // MAC tx callback protoype
static void XEmacPsErrorHandler(void *Callback, u8 Direction, u32 ErrorWord); // MAC err callback protoype
void EmacPsUtilErrorTrap(const char *Message); // 'Main' function error handler

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

 static inline s32 InitGic(XScuGic* IntcInstance, XScuGic_Config* CfgPtr, u32 BaseAddr) {
    u32 Status;
     // Initialize the GIC
    CfgPtr = XScuGic_LookupConfig(BaseAddr);
    Status =XScuGic_CfgInitialize(IntcInstance, CfgPtr, BaseAddr);

    if(Status != XST_SUCCESS) {
        EmacPsUtilErrorTrap("Error in configuring/initializing the GIC\n\r");
        return XST_FAILURE;
    }
    if(XScuGic_IsInitialized(BaseAddr)!= 1U) {
        EmacPsUtilErrorTrap("XScuGic_IsInitialized Failed\n\r");
        return XST_FAILURE;
    }
    if(XScuGic_SelfTest(IntcInstance)!= XST_SUCCESS) {
        EmacPsUtilErrorTrap("XScuGic_SelfTest Failed\n\r");
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

 static inline s32 InitEmac(XEmacPs* EmacInstance, XEmacPs_Config* CfgPtr, u32 BaseAddr) {
    u32 Status, GemVersion;
    XEmacPs_Bd BdTemplate;

     // Initialize the EMAC
    CfgPtr = XEmacPs_LookupConfig(BaseAddr);
    Status =XEmacPs_CfgInitialize(EmacInstance, CfgPtr, CfgPtr->BaseAddress);
    if(Status != XST_SUCCESS) {
        EmacPsUtilErrorTrap("Error in configuring/initializing the EMAC\n\r");
        return XST_FAILURE;
    }
    GemVersion = ((Xil_In32(CfgPtr->BaseAddress + 0xFC)) >> 16) & 0xFFF; // geting GEM version to see if we need additional clock setup

    // Set the MAC Address
    Status = XEmacPs_SetMacAddress(EmacInstance,EmacPsMAC,1);
    if(Status != XST_SUCCESS) {
        EmacPsUtilErrorTrap("Error in setting the MAC Address\n\r");
        return XST_FAILURE;
    }

    // Print the MAC PS Info
    xil_printf("EMAC.name=%s \n\r Emac.BAddr=0x%x\n\r Emac.PhyType=%s \n\r Emac.GemVersion=%0d\n\r Emac.MacAddr=%s Emac.CacheCoh=%0d",
    CfgPtr->Name, CfgPtr->BaseAddress, CfgPtr->IntrId, CfgPtr->PhyType,GemVersion,EmacPsMAC, CfgPtr->IsCacheCoherent);

    // Setup the interrupt callbacks
	Status = XEmacPs_SetHandler(EmacInstance,
				    XEMACPS_HANDLER_DMASEND,
				    (void *) XEmacPsSendHandler,
				    EmacInstance);
	Status |=
		XEmacPs_SetHandler(EmacInstance,
				   XEMACPS_HANDLER_DMARECV,
				   (void *) XEmacPsRecvHandler,
				   EmacInstance);
	Status |=
		XEmacPs_SetHandler(EmacInstance, XEMACPS_HANDLER_ERROR,
				   (void *) XEmacPsErrorHandler,
				   EmacInstance);
    
    // Setup the RX BD Ring
    XEmacPs_BdClear(&BdTemplate); //create a zero'ed out descriptor
    Status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing (EmacInstance)),
				      (UINTPTR) RxBdSpacePtr,
				      (UINTPTR) RxBdSpacePtr,
				      XEMACPS_BD_ALIGNMENT,
				      RXBD_CNT);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setting up RxBD space, BdRingCreate");
		return XST_FAILURE;
	}
    Status = XEmacPs_BdRingClone(&(XEmacPs_GetRxRing(EmacInstance)), &BdTemplate, XEMACPS_RECV);
    if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error clearing RxBD space, BdRingClone");
		return XST_FAILURE;
	}

     // Setup the TX BD Ring
	XEmacPs_BdClear(&BdTemplate);  //create a zero'ed out descriptor
	XEmacPs_BdSetStatus(&BdTemplate, XEMACPS_TXBUF_USED_MASK); //set Used bit in template
	Status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing
					(EmacInstance)),
				      (UINTPTR) TxBdSpacePtr,
				      (UINTPTR) TxBdSpacePtr,
				      XEMACPS_BD_ALIGNMENT,
				      TXBD_CNT);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setting up TxBD space, BdRingCreate");
		return XST_FAILURE;
	}
	Status = XEmacPs_BdRingClone(&(XEmacPs_GetTxRing(EmacInstance)), &BdTemplate, XEMACPS_SEND);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setting up TxBD space, BdRingClone");
		return XST_FAILURE;
	}
    return XST_SUCCESS;

 }

int main () {
    XScuGic_Config Gic_CfgPtr;
    XEmacPs_Config Emac_CfgPtr;

    u32 Status;

    print("Hello World\n\r");

    print("Initializing GIC_0\n\r");
    Status = InitGic(&IntcInstance, &Gic_CfgPtr, XPAR_XSCUGIC_0_BASEADDR);
    if(Status != XST_SUCCESS) {
        EmacPsUtilErrorTrap("Initialization of GIC failed\n\r");
    }

    print("Initializing the EMAC_0\n\r");
    Status = InitEmac(&EmacInstance, &Emac_CfgPtr, XPAR_XEMACPS_0_BASEADDR);
    if(Status != XST_SUCCESS) {
        EmacPsUtilErrorTrap("Initialization of EMAC failed\n\r");
    }

        

    print("Enabling the GIC interrupts\n\r");
    XScuGic_Enable(&IntcInstance, ETH0_INT_ID);
    XScuGic_Enable(&IntcInstance, ETH0_WAKE_INT_ID);

    print("Leaving Main");

return 0;
}


/****************************************************************************/
/**
*
* This the Transmit handler callback function and will increment a shared
* counter that can be shared by the main thread of operation.
*
* @param	Callback is the pointer to the instance of the EmacPs device.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XEmacPsSendHandler(void *Callback)
{
	XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;
	
    //Increment the counter
	TxCounter++;
}


/****************************************************************************/
/**
*
* This is the Receive handler callback function and will increment a shared
* counter that can be shared by the main thread of operation.
*
* @param	Callback is a pointer to the instance of the EmacPs device.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XEmacPsRecvHandler(void *Callback)
{
	XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

	/*
	 * Increment the counter so that main thread knows something
	 * happened.
	 */
	RxCounter++;
	if (EmacPsInstancePtr->Config.IsCacheCoherent == 0) {
		Xil_DCacheInvalidateRange((UINTPTR)&RxFrame, sizeof(EthernetFrame));
	}
}


/****************************************************************************/
/**
*
* This is the Error handler callback function and this function increments
* the error counter so that the main thread knows the number of errors.
*
* @param	Callback is the callback function for the driver. This
*		parameter is not used in this example.
* @param	Direction is passed in from the driver specifying which
*		direction error has occurred.
* @param	ErrorWord is the status register value passed in.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XEmacPsErrorHandler(void *Callback, u8 Direction, u32 ErrorWord)
{
	XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

	/*
	 * Increment the counter so that main thread knows something
	 * happened. Reset the device and reallocate resources ...
	 */
	DeviceErrors++;

	switch (Direction) {
		case XEMACPS_RECV:
			if (ErrorWord & XEMACPS_RXSR_HRESPNOK_MASK) {
				EmacPsUtilErrorTrap("Receive DMA error");
			}
			if (ErrorWord & XEMACPS_RXSR_RXOVR_MASK) {
				EmacPsUtilErrorTrap("Receive over run");
			}
			if (ErrorWord & XEMACPS_RXSR_BUFFNA_MASK) {
				EmacPsUtilErrorTrap("Receive buffer not available");
			}
			break;
		case XEMACPS_SEND:
			if (ErrorWord & XEMACPS_TXSR_HRESPNOK_MASK) {
				EmacPsUtilErrorTrap("Transmit DMA error");
			}
			if (ErrorWord & XEMACPS_TXSR_URUN_MASK) {
				EmacPsUtilErrorTrap("Transmit under run");
			}
			if (ErrorWord & XEMACPS_TXSR_BUFEXH_MASK) {
				EmacPsUtilErrorTrap("Transmit buffer exhausted");
			}
			if (ErrorWord & XEMACPS_TXSR_RXOVR_MASK) {
				EmacPsUtilErrorTrap("Transmit retry excessed limits");
			}
			if (ErrorWord & XEMACPS_TXSR_FRAMERX_MASK) {
				EmacPsUtilErrorTrap("Transmit collision");
			}
			if (ErrorWord & XEMACPS_TXSR_USEDREAD_MASK) {
				EmacPsUtilErrorTrap("Transmit buffer not available");
			}
			break;
	}
		// EmacPsResetDevice(EmacPsInstancePtr);
}

/****************************************************************************/
/**
*
* This function is called by example code when an error is detected. It
* can be set as a breakpoint with a debugger or it can be used to print out
* the given message if there is a UART or STDIO device.
*
* @param    Message is the text explaining the error
*
* @return   None
*
* @note     None
*
*****************************************************************************/
void EmacPsUtilErrorTrap(const char *Message)
{
	static u32 Count = 0;

	Count++;

#ifdef STDOUT_BASEADDRESS
	xil_printf("%s\r\n", Message);
#else
	(void) Message;
#endif
}