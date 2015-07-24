////////////////////////////////////////////////////////////////////////////////
//                   Mountain View Silicon Tech. Inc.
//		Copyright 2012, Mountain View Silicon Tech. Inc., ShangHai, China
//                   All rights reserved.
//
//		Filename	:startup.c
//
//		Changelog	:
//				2012-09-12 merge the TI demo and FPGA auto-test into a new one by Robert
//				2013-05-23 fix the standard library rt-lib initialization bug by Robert
//				2013-05-27 convert to C file to import extern .h macro definition by Robert
//              2014-04-23 add 32KHz external oscillator capacitance input/output calibration value by Robert
///////////////////////////////////////////////////////////////////////////////
//#include "app_config.h"
#include <stdio.h>
#define DBG printf

void reset_handler(void);
void nmi_handler(void);
void hardfault_handler(void);
void memmanage_handler(void);
void busfault_handler(void);
void usagefault_handler(void);

//static unsigned char sys_stack_heap[CFG_SYS_STACK_SIZE] __attribute__((section("MSP"))) = {0xA5,};

//static void (*const vect_table[])(void) __attribute__((section("EXCEPT_VECTS"))) = {
//    (void(*)(void))(0x20000000 + CFG_SYS_STACK_SIZE),	//   Top of Stack
//    reset_handler,				//#1 :Reset Handler
//    nmi_handler,				//#2 :NMI Handler
//    hardfault_handler,			//#3 :Hard Fault Handler
//    memmanage_handler,			//#4 :MPU Fault Handler
//    busfault_handler,			//#5 :Bus Fault Handler
//    usagefault_handler,			//#6 :Usage Fault Handler
//    0,							//#7 :Reserved
//    0,							//#8 :Reserved
//    0,							//#9 :Reserved
//    0,							//#10:Reserved
//    SVC_Handler,			//#11:SVCall Handler
//    0,							//#12:Debug Monitor Handler
//    0,							//#13:Reserved
//    PendSV_Handler,			//#14:PendSV Handler
//    SysTick_Handler,			//#15:SysTick Handler
//	

//    //-----------External Interrupts---------------------
//    GpioInterrupt, 				//#16: GPIO 
//    RtcInterrupt,				//#17: RTC
//    IrInterrupt,		    	//#18: IR
//    FuartInterrupt,				//#19: FUART
//    BuartInterrupt,				//#20: BUART 
//    PwcInterrupt,				//#21: PWC 
//    Timer0Interrupt,			//#22: TIMER0 
//    UsbInterrupt,				//#23: USB 
//    DmaCh0Interrupt,   			//#24: DMA CH0 
//    DmaCh1Interrupt,		    //#25: DMA CH1 
//    audio_decoder_interrupt_handler,      		//26: DECODER 
//    SpisInterrupt,              //#27: SPIS
//    SdInterrupt,				//#28: SDIO
//    SpimInterrupt,              //#29: SPIM
//    Timer1Interrupt,			//#30: TIMER1
//    WatchDogInterrupt,			//#31: WatchDog
//};
		
//******************************************************************************
//
// This is the code that gets called when the processor first starts execution
// following a reset event.
//
//******************************************************************************

/*
 * back trace
 */
/** \brief  Get ISPR Register

    This function returns the content of the ISPR Register.

    \return               ISPR Register value
 */
static __inline unsigned int __get_IPSR(void)
{
  register unsigned int __regIPSR          __asm("ipsr");
  return(__regIPSR);
} 
 
void trapfault_handler_dumpstack(unsigned long* irqs_regs, unsigned long* user_regs)
{
	DBG("\n>>>>>>>>>>>>>>[");
	switch(__get_IPSR())
	{
		case	3:
			DBG("Hard Fault");
			break;

		case	4:
			DBG("Memory Manage");
			break;

		case	5:
			DBG("Bus Fault");
			break;

		case	6:
			DBG("Usage Fault");
			break;

		default:
			DBG("Unknown Fault %d", __get_IPSR());
			break;
	}
	DBG(",corrupt,dump registers]>>>>>>>>>>>>>>>>>>\n");

	DBG("R0  = 0x%08X\n", irqs_regs[0]);
	DBG("R1  = 0x%08X\n", irqs_regs[1]);
	DBG("R2  = 0x%08X\n", irqs_regs[2]);
	DBG("R3  = 0x%08X\n", irqs_regs[3]);

	DBG("R4  = 0x%08X\n", user_regs[0]);
	DBG("R5  = 0x%08X\n", user_regs[1]);
	DBG("R6  = 0x%08X\n", user_regs[2]);
	DBG("R7  = 0x%08X\n", user_regs[3]);
	DBG("R8  = 0x%08X\n", user_regs[4]);
	DBG("R9  = 0x%08X\n", user_regs[5]);
	DBG("R10 = 0x%08X\n", user_regs[6]);
	DBG("R11 = 0x%08X\n", user_regs[7]);

	DBG("R12 = 0x%08X\n", irqs_regs[4]);
	DBG("SP  = 0x%08X\n", &irqs_regs[8]);
	DBG("LR  = 0x%08X\n", irqs_regs[5]);
	DBG("PC  = 0x%08X\n", irqs_regs[6]);
	DBG("PSR = 0x%08X\n", irqs_regs[7]);

	DBG("BFAR = 0x%08X\n", (*((volatile unsigned long*)(0xE000ED38))));
	DBG("CFSR = 0x%08X\n", (*((volatile unsigned long*)(0xE000ED28))));
	DBG("HFSR = 0x%08X\n", (*((volatile unsigned long*)(0xE000ED2C))));
	DBG("DFSR = 0x%08X\n", (*((volatile unsigned long*)(0xE000ED30))));
	DBG("AFSR = 0x%08X\n", (*((volatile unsigned long*)(0xE000ED3C))));
	//DBG("Terminated@%u ms\n", auxtmr_count_get());
	/*
	#ifdef	DEBUG
	if(*(unsigned long*)0x20000000 != 0xA5A5A5A5)
	{
		DBG("Error:System Stack Overflow\n");
		return;
	}
	#endif //DEBUG
	*/
	/*
	 * indefinitely deadloop
	 */
	while(1);;;
}

__asm void __bootup_stubs(void)
{
    
Stack_Size      EQU     0x00001000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

//; <h> Heap Configuration
//;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
//; </h>

Heap_Size       EQU     0x00001000


                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB

//Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp               // Top of Stack
                DCD     reset_handler              // Reset Handler
                DCD     nmi_handler                // NMI Handler
                DCD     hardfault_handler          // Hard Fault Handler
                DCD     memmanage_handler          // MPU Fault Handler
                DCD     busfault_handler           // Bus Fault Handler
                DCD     usagefault_handler         // Usage Fault Handler
                DCD     0                          //Reserved
                DCD     0                          // Reserved
                DCD     0                          // Reserved
                DCD     0                          // Reserved
                DCD     SVC_Handler                // SVCall Handler
                DCD     0                           // Debug Monitor Handler
                DCD     0                          // Reserved
                DCD     PendSV_Handler             // PendSV Handler
                DCD     SysTick_Handler            // SysTick Handler

                // External Interrupts
                DCD     GpioInterrupt                   // Window WatchDog                                        
                DCD     RtcInterrupt                   // PVD through EXTI Line detection                        
                DCD     IrInterrupt             // Tamper and TimeStamps through the EXTI line            
                DCD     FuartInterrupt                      // RTC Wakeup through the EXTI line                       
                DCD     BuartInterrupt                  // FLASH                                           
                DCD     PwcInterrupt                    // RCC                                             
                DCD     Timer0Interrupt                  // EXTI Line0                                             
                DCD     UsbInterrupt                  //EXTI Line1                                             
                DCD     DmaCh0Interrupt                  // EXTI Line2                                             
                DCD     DmaCh1Interrupt                  // EXTI Line3                                             
                DCD     audio_decoder_interrupt_handler                  //EXTI Line4                                             
                DCD     SpisInterrupt          // DMA1 Stream 0                                   
                DCD     SdInterrupt           // DMA1 Stream 1                                   
                DCD     SpimInterrupt           // DMA1 Stream 2                                   
                DCD     Timer1Interrupt           // DMA1 Stream 3                                   
                DCD     WatchDogInterrupt           // DMA1 Stream 4                                   
                                 
__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

  
	//image total size in bytes
	AREA    |.ARM.__at_0x00000080|, CODE, READONLY
IMAGE_SIZE	DCD	0xFFFFFFFF

	//product ID & Check sum
	AREA    |.ARM.__at_0x00000084|, CODE, READONLY
PIDCHKSUM	FILL 8, 0xFF


	//constant data offset
	AREA    |.ARM.__at_0x0000008C|, CODE, READONLY
CONSTDATA	DCD	0xA000

	//user data offset
	AREA    |.ARM.__at_0x00000090|, CODE, READONLY
USERDATA	DCD	0x1A0000

	//sdk version
	AREA    |.ARM.__at_0x00000094|, CODE, READONLY
SDK_VER_CHIPID	DCB		0x12    //MV chip ID
SDK_VER_MAJOR	DCB		2       //MV SDK major version
SDK_VER_MINOR	DCB		0      //MV SDK minor version
SDK_VER_USER	DCB		1       //user SDK version

	//code memory(exclude of this 4 bytes) crc
	AREA    |.ARM.__at_0x00000098|, CODE, READONLY
SDK_CODECRC	DCD	0xFFFFFFFF

    //code magic number
	AREA    |.ARM.__at_0x0000009C|, CODE, READONLY
SDK_CODEMGC	DCD	0xB0BEBDC9

    //32KHz external oscillator input/output capacitance calibration value
	AREA    |.ARM.__at_0x000000A0|, CODE, READONLY
OSC32K_CAP	DCD	0x00000706

    AREA    |.ARM.__at_0x000000A4|, CODE, READONLY    
USER_RESV_ZOOM    FILL  (0xFC - 0xA4),    0xFF

    AREA    |.ARM.__at_0x000000FC|, CODE, READONLY    
CODE_ENCRYP_FLAG	DCD  0xFFFFFFFF

	AREA    |.text|, CODE, READONLY

	IMPORT	|Region$$Table$$Base|
	IMPORT	|Region$$Table$$Limit|

	IMPORT  __main
	IMPORT  __low_level_init
#ifdef	FREERTOS_VERSION
	IMPORT	mmm_pool_top 
#endif

		//the follow code crack the uvision startup code -huangyucai20111018
__mv_main
		//set up the system stack	
//#ifdef	FREERTOS_VERSION
//		LDR		SP,=mmm_pool_top 
//#else
//		LDR		SP,=0x20008000
//#endif //FREERTOS_VERSION 
//		SUB		SP,SP,#0x1000
//		LDR		R0,=__initial_sp
//		SUB		R0,R0,#CFG_SYS_STACK_SIZE
//		LDR		R0,[R0]
//		PUSH	{R0}

mv_main	PROC
		EXPORT	mv_main	
			
//		//get the load region layout table
//		LDR		R4,=|Region$$Table$$Base|
//		LDR		R5,=|Region$$Table$$Limit|
//__NEXT_REGION
//		CMP		R4,R5
//		//everything is ok
//		BCS		__REGION_DECOMP_OK
//		LDM		R4,{R0-R3}
//		//decompress the data following the compress algorithm as compiling method
//		STMDB	R13!,{R4,R5}
//		ORR		R3,R3,#0x01
//		BLX 	R3
//		LDMIA	R13!,{R4,R5}
//		ADD		R4,R4,#0x10
//		B		__NEXT_REGION

//__REGION_DECOMP_OK
//#ifdef __MICROLIB
		/*
		 * TO DO
		 */
//#else
		//IMPORT	__rt_lib_init 
		//BL		__rt_lib_init
//#endif//__MICROLIB

		//fill the system stack space with debug symbol for debug only -huangyucai20111121
//#ifdef CFG_SHELL_DEBUG
//		LDR		R2,=CFG_SYS_STACK_SIZE
//		LDR		R3,=__initial_sp
//		SUB		R3,R3,#1
//		MOV		R4,#0xA5
//AGAIN
//		STRB	R4,[R3],#-0x01
//		SUBS	R2,R2,#0x01
//		BHI		AGAIN
//#endif //CFG_SHELL_DEBUG
        
//        CPSIE   I
//        CPSIE   F
		LDR		SP,=__initial_sp
    ADD   LR, PC, #0x6
		LDR		R0,=__low_level_init
		BLX		R0
		LDR		R0,=__main
		BX		R0

		ENDP
		ALIGN

reset_handler PROC
		EXPORT  reset_handler               [WEAK]					    
		MOV		R0,R0
		LDR		R0,=__mv_main
		BX    	R0
		ENDP

//;Dummy Exception Handlers (infinite loops which can be modified) 
nmi_handler PROC
        EXPORT  nmi_handler					[WEAK]					    
        B       .
        ENDP

hardfault_handler PROC
        EXPORT  hardfault_handler			[WEAK]					    
//#ifdef FREERTOS_VERSION
		IMPORT trapfault_handler_dumpstack
		TST		LR,	#4
		MRSEQ	R0,	MSP
		MRSNE	R0,	PSP
		PUSH	{R4-R11}
		MRS		R1,	MSP
		B		trapfault_handler_dumpstack
//#else
//		B		.
//#endif //(FREERTOS_VERSION))
		ENDP
 
memmanage_handler PROC
        EXPORT  memmanage_handler			[WEAK]					    
#ifdef FREERTOS_VERSION
		IMPORT trapfault_handler_dumpstack
		TST		LR,	#4
		MRSEQ	R0,	MSP
		MRSNE	R0,	PSP
		PUSH	{R4-R11}
		MRS		R1,	MSP
		B		trapfault_handler_dumpstack
#else
		B		.
#endif //(FREERTOS_VERSION))
        ENDP

busfault_handler PROC
        EXPORT  busfault_handler			[WEAK]					    
#ifdef FREERTOS_VERSION
		IMPORT trapfault_handler_dumpstack
		TST		LR,	#4
		MRSEQ	R0,	MSP
		MRSNE	R0,	PSP
		PUSH	{R4-R11}
		MRS		R1,	MSP
		B		trapfault_handler_dumpstack
#else
		B		.
#endif //(FREERTOS_VERSION))
        ENDP

usagefault_handler PROC
        EXPORT  usagefault_handler			[WEAK]					    
#ifdef FREERTOS_VERSION
		IMPORT trapfault_handler_dumpstack
		TST		LR,	#4
		MRSEQ	R0,	MSP
		MRSNE	R0,	PSP
		PUSH	{R4-R11}
		MRS		R1,	MSP
		B		trapfault_handler_dumpstack
#else
		B		.
#endif //(FREERTOS_VERSION))
		ENDP


Default_Handler PROC
                EXPORT     SVC_Handler              [WEAK]
                EXPORT     PendSV_Handler           [WEAK]
                EXPORT     SysTick_Handler              [WEAK]
                EXPORT     GpioInterrupt             [WEAK]         // Window WatchDog                                        
                EXPORT     RtcInterrupt               [WEAK]       // PVD through EXTI Line detection                        
                EXPORT     IrInterrupt             [WEAK]   // Tamper and TimeStamps through the EXTI line            
                EXPORT     FuartInterrupt           [WEAK]             // RTC Wakeup through the EXTI line                       
                EXPORT     BuartInterrupt           [WEAK]          // FLASH                                           
                EXPORT     PwcInterrupt             [WEAK]          // RCC                                             
                EXPORT     Timer0Interrupt            [WEAK]         // EXTI Line0                                             
                EXPORT     UsbInterrupt              [WEAK]       //EXTI Line1                                             
                EXPORT     DmaCh0Interrupt            [WEAK]         // EXTI Line2                                             
                EXPORT     DmaCh1Interrupt              [WEAK]       // EXTI Line3                                             
                EXPORT     audio_decoder_interrupt_handler   [WEAK]                  //EXTI Line4                                             
                EXPORT     SpisInterrupt          [WEAK]   // DMA1 Stream 0                                   
                EXPORT     SdInterrupt           [WEAK]   // DMA1 Stream 1                                   
                EXPORT     SpimInterrupt          [WEAK]    // DMA1 Stream 2                                   
                EXPORT     Timer1Interrupt           [WEAK]   // DMA1 Stream 3                                   
                EXPORT     WatchDogInterrupt          [WEAK]    // DMA1 Stream 4  

SVC_Handler
PendSV_Handler
SysTick_Handler
GpioInterrupt                                     
RtcInterrupt                                 
IrInterrupt            
FuartInterrupt                        
BuartInterrupt                                       
PwcInterrupt                                                 
Timer0Interrupt                                              
UsbInterrupt                                             
DmaCh0Interrupt                                             
DmaCh1Interrupt                                           
audio_decoder_interrupt_handler                                       
SpisInterrupt                        
SdInterrupt                                    
SpimInterrupt                                     
Timer1Interrupt                                       
WatchDogInterrupt         
                B       .

                ENDP

		ALIGN

//******************************************************************************
//
// Some code in the normal code section for initializing the heap and stack.
//
//******************************************************************************
//
//
//******************************************************************************
//
// The function expected of the C library startup code for defining the stack
// and heap memory locations.  For the C library version of the startup code,
// provide this function so that the C library initialization code can find out
// the location of the stack and heap.
// 
//******************************************************************************

                
                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit
                 EXPORT  Heap_Mem
                 EXPORT  Stack_Mem

                 
__user_initial_stackheap

                 LDR     R0, =  Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

//******************************************************************************
//
// Make sure the end of this section is aligned.
//
//******************************************************************************
       
//
//******************************************************************************
//
// Tell the assembler that we're done.
//
//******************************************************************************

        END
}


