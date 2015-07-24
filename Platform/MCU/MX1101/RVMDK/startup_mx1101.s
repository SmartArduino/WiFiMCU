;******************************************************************************
;* @file    startup_mx1101.s 
;* @author  William Xu
;* @version V1.0.0
;* @date    05-May-2014
;* @Brief   MX1101 devices vector table for MDK-ARM toolchain. 
;*                      This module performs:
;*                      - Set the initial SP
;*                      - Set the initial PC == Reset_Handler
;*                      - Set the vector table entries with the exceptions ISR address
;*                      - Branches to __main in the C library (which eventually
;*                        calls main()).
;*                      After Reset the CortexM3 processor is in Thread mode,
;*                      priority is Privileged, and the Stack is set to Main.
;* <<< Use Configuration Wizard in Context Menu >>>   
;******************************************************************************
;* @attention
;*
;* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
;* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
;* TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
;* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
;* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
;* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
;*
;******************************************************************************

; Amount of memory (in bytes) allocated for Stack
; Tailor this value to your application needs
; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000400

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00012000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit

                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset
                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size
                IMPORT  vPortSVCHandler
                IMPORT  xPortPendSVHandler
                IMPORT  xPortSysTickHandler
					
__Vectors       DCD     __initial_sp               			; Top of Stack
                DCD     Reset_Handler              			; Reset Handler
                DCD     NMI_Handler                			; NMI Handler
                DCD     HardFault_Handler          			; Hard Fault Handler
                DCD     MemManage_Handler          			; MPU Fault Handler
                DCD     BusFault_Handler           			; Bus Fault Handler
                DCD     UsageFault_Handler         			; Usage Fault Handler
                DCD     0                          			; Reserved
                DCD     0                          			; Reserved
                DCD     0                          			; Reserved
                DCD     0                          			; Reserved
                DCD     vPortSVCHandler            			; SVCall Handler
                DCD     0                          			; Debug Monitor Handler
                DCD     0                          			; Reserved
                DCD     xPortPendSVHandler         			; PendSV Handler
                DCD     SysTick_Handler            			; SysTick Handler

                ; External Interrupts
                DCD     GpioInterrupt              			; Window WatchDog                                        
                DCD     RtcInterrupt               			; PVD through EXTI Line detection                        
                DCD     IrInterrupt                			; Tamper and TimeStamps through the EXTI line            
                DCD     FuartInterrupt             			; RTC Wakeup through the EXTI line                       
                DCD     BuartInterrupt             			; FLASH                                           
                DCD     PwcInterrupt               			; RCC                                             
                DCD     Timer0Interrupt            			; EXTI Line0                                             
                DCD     UsbInterrupt               			; Line1                                             
                DCD     DmaCh0Interrupt            			; EXTI Line2                                             
                DCD     DmaCh1Interrupt           			; EXTI Line3                                             
                DCD     audio_decoder_interrupt_handler     ; EXTI Line4                                             
                DCD     SpisInterrupt          				; DMA1 Stream 0                                   
                DCD     SdInterrupt           				; DMA1 Stream 1                                   
                DCD     SpimInterrupt           			; DMA1 Stream 2                                   
                DCD     Timer1Interrupt           			; DMA1 Stream 3                                   
                DCD     WatchDogInterrupt           		; DMA1 Stream 4                                   
                                 
__Vectors_End

__Vectors_Size  EQU  __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

; Reset handler
Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
        		IMPORT  __low_level_init
        		IMPORT  __main

                LDR     R0, =__low_level_init
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler                [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                IMPORT  hard_fault_handler_c
                TST LR, #4
                ITE EQ
                MRSEQ R0, MSP
                MRSNE R0, PSP
                B       hard_fault_handler_c
                ENDP								   
MemManage_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                IMPORT  hard_fault_handler_c
                TST LR, #4
                ITE EQ
                MRSEQ R0, MSP
                MRSNE R0, PSP
                B       hard_fault_handler_c
                ENDP		
BusFault_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                IMPORT  hard_fault_handler_c
                TST LR, #4
                ITE EQ
                MRSEQ R0, MSP
                MRSNE R0, PSP
                B       hard_fault_handler_c
                ENDP		
UsageFault_Handler\
                PROC
                EXPORT  HardFault_Handler          [WEAK]
                IMPORT  hard_fault_handler_c
                TST LR, #4
                ITE EQ
                MRSEQ R0, MSP
                MRSNE R0, PSP
                B       hard_fault_handler_c
                ENDP		
SVC_Handler     PROC
                EXPORT  SVC_Handler                [WEAK]
				IMPORT vPortSVCHandler
                B vPortSVCHandler
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler           [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler             [WEAK]
				IMPORT xPortPendSVHandler
                B xPortPendSVHandler
                ENDP

Default_Handler PROC
                EXPORT     SysTick_Handler              	[WEAK]
                EXPORT     GpioInterrupt             		[WEAK]                                           
                EXPORT     RtcInterrupt               		[WEAK]                            
                EXPORT     IrInterrupt             			[WEAK]           
                EXPORT     FuartInterrupt           		[WEAK]                                  
                EXPORT     BuartInterrupt           		[WEAK]                                            
                EXPORT     PwcInterrupt             		[WEAK]                                                
                EXPORT     Timer0Interrupt            		[WEAK]                                                
                EXPORT     UsbInterrupt              		[WEAK]                                       
                EXPORT     DmaCh0Interrupt            		[WEAK]                                              
                EXPORT     DmaCh1Interrupt              	[WEAK]                                            
                EXPORT     audio_decoder_interrupt_handler  [WEAK]                                 
                EXPORT     SpisInterrupt          			[WEAK]                           
                EXPORT     SdInterrupt           			[WEAK]                              
                EXPORT     SpimInterrupt          			[WEAK]                                
                EXPORT     Timer1Interrupt           		[WEAK]                                   
                EXPORT     WatchDogInterrupt          		[WEAK]    

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

;*******************************************************************************
; User Stack and Heap initialization
;*******************************************************************************
                 IF      :DEF:__MICROLIB
                
                 EXPORT  __initial_sp
                 EXPORT  __heap_base
                 EXPORT  __heap_limit
                
                 ELSE
                
                 IMPORT  __use_two_region_memory
                 EXPORT  __user_initial_stackheap
                 EXPORT  Heap_Mem
                 EXPORT  Stack_Mem
                 
__user_initial_stackheap

                 LDR     R0, =  Heap_Mem
                 LDR     R1, =(Stack_Mem + Stack_Size)
                 LDR     R2, = (Heap_Mem +  Heap_Size)
                 LDR     R3, = Stack_Mem
                 BX      LR

                 ALIGN

                 ENDIF

                 END

;************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE*****
