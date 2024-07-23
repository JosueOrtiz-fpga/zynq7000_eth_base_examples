/*
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION 
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * 
 *
 * This file is a generated sample test application.
 *
 * This application is intended to test and/or illustrate some 
 * functionality of your system.  The contents of this file may
 * vary depending on the IP in your system and may use existing
 * IP driver functions.  These drivers will be generated in your
 * SDK application project when you run the "Generate Libraries" menu item.
 *
 */


#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "scugic_header.h"
#include "xemacps.h"
#include "xemacps_example.h"
#include "emacps_header.h"









int main() 
{

   static XScuGic intc;
   static XEmacPs ps7_ethernet_0;

   Xil_ICacheEnable();
   Xil_DCacheEnable();

   print("---Entering main---\n\r");

   

   {
      int Status;
      
      print("\r\n Running ScuGicSelfTestExample() for ps7_scugic_0...\r\n");
      
      Status = ScuGicSelfTestExample(XPAR_PS7_SCUGIC_0_DEVICE_ID);
      
      if (Status == 0) {
         print("ScuGicSelfTestExample PASSED\r\n");
      }
      else {
         print("ScuGicSelfTestExample FAILED\r\n");
      }
   } 
	
   {
       int Status;

       Status = ScuGicInterruptSetup(&intc, XPAR_PS7_SCUGIC_0_DEVICE_ID);
       if (Status == 0) {
          print("ScuGic Interrupt Setup PASSED\r\n");
       } 
       else {
         print("ScuGic Interrupt Setup FAILED\r\n");
      } 
   }
   
   {
      int Status;

      print("\r\n Running PPU Test using ps7_ethernet_0...\r\n");
      
      Status = EmacPsDmaIntrExample(&intc, &ps7_ethernet_0, \
                                 XPAR_PS7_ETHERNET_0_DEVICE_ID, \
                                 XPAR_PS7_ETHERNET_0_INTR);
	
      if (Status == 0) {
         print("EmacPsDmaIntrExample PASSED\r\n");
      } 
      else {
         print("EmacPsDmaIntrExample FAILED\r\n");
      }

   }


   print("---Exiting main---\n\r");

   Xil_DCacheDisable();
   Xil_ICacheDisable();

   return 0;
}

