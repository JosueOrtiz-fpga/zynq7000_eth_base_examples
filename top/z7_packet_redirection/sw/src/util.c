#include "util.h"
#include "sleep.h"

#define PHY_DETECT_REG1 2
#define PHY_DETECT_REG2 3

#define PHY_ID_MARVELL	0x141
#define PHY_ID_TI		0x2000

/****************************************************************************/
/**
*
* Set the MAC addresses in the frame.
*
* @param    FramePtr is the pointer to the frame.
* @param    DestAddr is the Destination MAC address.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameHdrFormatMAC(EthernetFrame *FramePtr, char *DestAddr)
{
	char *Frame = (char *) FramePtr;
	char *SourceAddress = EmacPsMAC;
	s32 Index;

	/* Destination address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *DestAddr++;
	}

	/* Source address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *SourceAddress++;
	}
}

/****************************************************************************/
/**
*
* Set the frame type for the specified frame.
*
* @param    FramePtr is the pointer to the frame.
* @param    FrameType is the Type to set in frame.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameHdrFormatType(EthernetFrame *FramePtr, u16 FrameType)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12;
	/*
	 * Do endian swap from little to big-endian.
	 */
	FrameType = Xil_EndianSwap16(FrameType);
	/*
	 * Set the type
	 */
	*(u16 *) Frame = FrameType;
}

/****************************************************************************/
/**
* This function places a pattern in the payload section of a frame. The pattern
* is a  8 bit incrementing series of numbers starting with 0.
* Once the pattern reaches 256, then the pattern changes to a 16 bit
* incrementing pattern:
* <pre>
*   0, 1, 2, ... 254, 255, 00, 00, 00, 01, 00, 02, ...
* </pre>
*
* @param    FramePtr is a pointer to the frame to change.
* @param    PayloadSize is the number of bytes in the payload that will be set.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameSetPayloadData(EthernetFrame *FramePtr, u32 PayloadSize)
{
	u32 BytesLeft = PayloadSize;
	u8 *Frame;
	u16 Counter = 0;

	/*
	 * Set the frame pointer to the start of the payload area
	 */
	Frame = (u8 *) FramePtr + XEMACPS_HDR_SIZE;

	/*
	 * Insert 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		*Frame++ = (u8) Counter++;
		BytesLeft--;
	}

	/*
	 * Switch to 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		*Frame++ = (u8) (Counter >> 8);	/* high */
		BytesLeft--;

		if (!BytesLeft) {
			break;
		}

		*Frame++ = (u8) Counter++;	/* low */
		BytesLeft--;
	}
}

/****************************************************************************/
/**
* This function verifies the frame data against a CheckFrame.
*
* Validation occurs by comparing the ActualFrame to the header of the
* CheckFrame. If the headers match, then the payload of ActualFrame is
* verified for the same pattern Util_FrameSetPayloadData() generates.
*
* @param    CheckFrame is a pointer to a frame containing the 14 byte header
*           that should be present in the ActualFrame parameter.
* @param    ActualFrame is a pointer to a frame to validate.
*
* @return   XST_SUCCESS if successful, else XST_FAILURE.
*
* @note     None.
*****************************************************************************/
LONG EmacPsUtilFrameVerify(EthernetFrame *CheckFrame,
			   EthernetFrame *ActualFrame)
{
	char *CheckPtr = (char *) CheckFrame;
	char *ActualPtr = (char *) ActualFrame;
	u16 BytesLeft;
	u16 Counter;
	u32 Index;

	/*
	 * Compare the headers
	 */
	for (Index = 0; Index < XEMACPS_HDR_SIZE; Index++) {
		if (CheckPtr[Index] != ActualPtr[Index]) {
			return XST_FAILURE;
		}
	}

	/*
	 * Get the length of the payload
	 */
	BytesLeft = *(u16 *) &ActualPtr[12];
	/*
	 * Do endian swap from big back to little-endian.
	 */
	BytesLeft = Xil_EndianSwap16(BytesLeft);
	/*
	 * Validate the payload
	 */
	Counter = 0;
	ActualPtr = &ActualPtr[14];

	/*
	 * Check 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		if (*ActualPtr++ != (char) Counter++) {
			return XST_FAILURE;
		}
		BytesLeft--;
	}

	/*
	 * Check 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		if (*ActualPtr++ != (char) (Counter >> 8)) {	/* high */
			return XST_FAILURE;
		}

		BytesLeft--;

		if (!BytesLeft) {
			break;
		}

		if (*ActualPtr++ != (char) Counter++) {	/* low */
			return XST_FAILURE;
		}

		BytesLeft--;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* This function sets all bytes of a frame to 0.
*
* @param    FramePtr is a pointer to the frame itself.
*
* @return   None.
*
* @note     None.
*
*****************************************************************************/
void EmacPsUtilFrameMemClear(EthernetFrame *FramePtr)
{
	u32 *Data32Ptr = (u32 *) FramePtr;
	u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);

	/* frame should be an integral number of words */
	while (WordsLeft--) {
		*Data32Ptr++ = 0xDEADBEEF;
	}
}


/****************************************************************************/
/**
*
* This function copies data from source to desitnation for n bytes.
*
* @param    Destination is the targeted string to copy to.
* @param    Source is the source string to copy from.
* @param    n is number of bytes to be copied.
*
* @note     This function is similar to strncpy(), however strncpy will
*           stop either at null byte or n bytes is been copied.
*           This function will copy n bytes without checking the content.
*
*****************************************************************************/
void EmacPsUtilstrncpy(char *Destination, const char *Source, u32 n)
{
	do {
		*Destination++ = *Source++;
	}
	while (--n != 0);
}



/****************************************************************************/
/**
*
* This function provides delays in seconds
*
* @param    delay in seconds
*
* @return   None.
*
* @note     for microblaze the delay is not accurate and it need to tuned.
*
*****************************************************************************/
void EmacpsDelay(u32 delay)
{
	sleep(delay);
}

/****************************************************************************/
/**
*
* This function enables the PHY Loopback
*
* @param    EmacPsInstancePtr - pointer to EmacPs instance
* @param    Speed - 10, 100, 1000
*
* @return   Status
*           XSCT_SUCESS - Pass
*           XSCT_FAILURE - Fail
*
*
*****************************************************************************/

LONG EmacPsUtilEnterLoopback(XEmacPs *EmacPsInstancePtr, u32 Speed)
{
	LONG Status;
	u16 PhyIdentity;
	u32 PhyAddr;

	/*
	 * Detect the PHY address
	 */
	PhyAddr = XEmacPsDetectPHY(EmacPsInstancePtr);

	if (PhyAddr >= 32) {
		EmacPsUtilErrorTrap("Error detect phy");
		return XST_FAILURE;
	}

	XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_DETECT_REG1, &PhyIdentity);

	if (PhyIdentity == PHY_ID_MARVELL) {
		Status = EmacPsUtilMarvellPhyLoopback(EmacPsInstancePtr, Speed, PhyAddr);
        xil_printf("PHY is MARVELL");
	}

	if (PhyIdentity == PHY_ID_TI) {
		Status = EmacPsUtilTiPhyLoopback(EmacPsInstancePtr, Speed, PhyAddr);
        xil_printf("PHY is TI");
	}

	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
/****************************************************************************/
/**
*
* This function detects and returns the Emac PHY address
*
* @param    EmacPsInstancePtr - pointer to EmacPs instance
*
* @return   PhyAddr
*
*
*****************************************************************************/

u32 XEmacPsDetectPHY(XEmacPs *EmacPsInstancePtr)
{
	u32 PhyAddr;
	u32 Status;
	u16 PhyReg1;
	u16 PhyReg2;

	for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++) {
		Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					 PHY_DETECT_REG1, &PhyReg1);

		Status |= XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					  PHY_DETECT_REG2, &PhyReg2);

		if ((Status == XST_SUCCESS) &&
		    (PhyReg1 > 0x0000) && (PhyReg1 < 0xffff) &&
		    (PhyReg2 > 0x0000) && (PhyReg2 < 0xffff)) {
			/* Found a valid PHY address */
			return PhyAddr;
		}
	}

	return PhyAddr;		/* default to 32(max of iteration) */
}
/****************************************************************************/
/**
*
* This function sets the PHY to loopback mode.
*
* @param    The XEMACPS driver instance
* @param    Speed is the loopback speed 10/100 Mbit.
*
* @return   XST_SUCCESS if successful, else XST_FAILURE.
*
* @note     None.
*
*****************************************************************************/
#define PHY_REG0_RESET    0x8000
#define PHY_REG0_LOOPBACK 0x4000
#define PHY_REG0_10       0x0100
#define PHY_REG0_100      0x2100
#define PHY_REG0_1000     0x0140
#define PHY_REG21_10      0x0030
#define PHY_REG21_100     0x2030
#define PHY_REG21_1000    0x0070
#define PHY_LOOPCR		0xFE
#define PHY_REGCR		0x0D
#define PHY_ADDAR		0x0E
#define PHY_RGMIIDCTL	0x86
#define PHY_RGMIICTL	0x32

#define PHY_REGCR_ADDR	0x001F
#define PHY_REGCR_DATA	0x401F

/* RGMII RX and TX tuning values */
#define PHY_TI_RGMII_ZCU102	0xA8
#define PHY_TI_RGMII_VERSALEMU	0xAB

LONG EmacPsUtilMarvellPhyLoopback(XEmacPs *EmacPsInstancePtr, u32 Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0;
	u16 PhyReg21  = 0;
	u16 PhyReg22  = 0;

	/*
	 * Setup speed and duplex
	 */
	switch (Speed) {
		case 10:
			PhyReg0 |= PHY_REG0_10;
			PhyReg21 |= PHY_REG21_10;
			break;
		case 100:
			PhyReg0 |= PHY_REG0_100;
			PhyReg21 |= PHY_REG21_100;
			break;
		case 1000:
			PhyReg0 |= PHY_REG0_1000;
			PhyReg21 |= PHY_REG21_1000;
			break;
		default:
			EmacPsUtilErrorTrap("Error: speed not recognized ");
			return XST_FAILURE;
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/*
	 * Switching to PAGE2
	 */
	PhyReg22 = 0x2;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 22, PhyReg22);

	/*
	 * Adding Tx and Rx delay. Configuring loopback speed.
	 */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 21, PhyReg21);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 21, &PhyReg21);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setting Reg 21 in Page 2");
		return XST_FAILURE;
	}
	/*
	 * Switching to PAGE0
	 */
	PhyReg22 = 0x0;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 22, PhyReg22);

	/*
	 * Issue a reset to phy
	 */
	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	PhyReg0 |= PHY_REG0_RESET;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	}

	/*
	 * Enable loopback
	 */
	PhyReg0 |= PHY_REG0_LOOPBACK;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	EmacpsDelay(1);

	return XST_SUCCESS;
}

LONG EmacPsUtilTiPhyLoopback(XEmacPs *EmacPsInstancePtr, u32 Speed, u32 PhyAddr)
{
	LONG Status;
	u16 PhyReg0  = 0, LoopbackSpeed = 0;
	u16 RgmiiTuning = PHY_TI_RGMII_ZCU102;

	/*
	 * Setup speed and duplex
	 */
	switch (Speed) {
		case 10:
			PhyReg0 |= PHY_REG0_10;
			break;
		case 100:
			PhyReg0 |= PHY_REG0_100;
			break;
		case 1000:
			PhyReg0 |= PHY_REG0_1000;
			break;
		default:
			EmacPsUtilErrorTrap("Error: speed not recognized ");
			return XST_FAILURE;
	}
	LoopbackSpeed = PhyReg0;

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	/*
	 * Make sure new configuration is in effect
	 */
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 1, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* Write PHY_LOOPCR */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_LOOPCR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, 0xEF20);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* Read PHY_LOOPCR */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_LOOPCR);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* SW reset */
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x1F, 0x4000);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy speed");
		return XST_FAILURE;
	}

	/* issue a reset to phy */
	Status  = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	PhyReg0 |= PHY_REG0_RESET;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error reset phy");
		return XST_FAILURE;
	}

	EmacpsDelay(1);

	/* enable loopback */
	PhyReg0 = LoopbackSpeed | PHY_REG0_LOOPBACK;
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0, PhyReg0);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, 0x10, 0x5048);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x10, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	/* Write to PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	// if ((Platform & PLATFORM_MASK_VERSAL) == PLATFORM_VERSALEMU) {
	// 	RgmiiTuning = PHY_TI_RGMII_VERSALEMU;
	// }
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, RgmiiTuning);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIIDCTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIIDCTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Write PHY_RGMIICTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, 0xD3);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	/* Read PHY_RGMIICTL */
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_ADDR);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, PHY_RGMIICTL);
	XEmacPs_PhyWrite(EmacPsInstancePtr, PhyAddr, PHY_REGCR, PHY_REGCR_DATA);
	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_ADDAR, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error in tuning");
		return XST_FAILURE;
	}

	Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, 0x11, &PhyReg0);
	if (Status != XST_SUCCESS) {
		EmacPsUtilErrorTrap("Error setup phy loopback");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
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