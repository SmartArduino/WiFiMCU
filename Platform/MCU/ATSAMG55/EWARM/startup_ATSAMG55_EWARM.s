;/**
; * \file
; *
; * \brief Startup file for SAMG55.
; *
; * Copyright (c) 2014 Atmel Corporation. All rights reserved.
; *
; * \asf_license_start
; *
; * \page License
; *
; * Redistribution and use in source and binary forms, with or without
; * modification, are permitted provided that the following conditions are met:
; *
; * 1. Redistributions of source code must retain the above copyright notice,
; *    this list of conditions and the following disclaimer.
; *
; * 2. Redistributions in binary form must reproduce the above copyright notice,
; *    this list of conditions and the following disclaimer in the documentation
; *    and/or other materials provided with the distribution.
; *
; * 3. The name of Atmel may not be used to endorse or promote products derived
; *    from this software without specific prior written permission.
; *
; * 4. This software may only be redistributed and used in connection with an
; *    Atmel microcontroller product.
; *
; * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
; * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
; * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
; * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
; * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
; * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
; * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
; * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
; * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; * POSSIBILITY OF SUCH DAMAGE.
; *
; * \asf_license_stop
; *
; */
; /**
; * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
; */
;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        EXTERN  vPortSVCHandler
        EXTERN  xPortPendSVHandler
        EXTERN  hard_fault_handler_c
        EXTERN  xPortSysTickHandler
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler             ; Reset Handler

        DCD     NMI_Handler               ; NMI Handler
        DCD     HardFault_Handler         ; Hard Fault Handler
        DCD     MemManage_Handler         ; MPU Fault Handler
        DCD     BusFault_Handler          ; Bus Fault Handler
        DCD     UsageFault_Handler        ; Usage Fault Handler
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     vPortSVCHandler           ; SVCall Handler
        DCD     DebugMon_Handler          ; Debug Monitor Handler
        DCD     0                         ; Reserved
        DCD     xPortPendSVHandler        ; PendSV Handler
        DCD     xPortSysTickHandler       ; SysTick Handler

         ; External Interrupts
        DCD     SUPC_Handler                      ; 0  Supply Controller                            
        DCD     RSTC_Handler                      ; 1  Reset Controller                      
        DCD     RTC_Handler                       ; 2  Real Time Clock           
        DCD     RTT_Handler                       ; 3  Real Time Timer                    
        DCD     WDT_Handler                       ; 4  Watchdog Timer                              
        DCD     PMC_Handler                       ; 5  Power Management                                   
        DCD     EFC_Handler                       ; 6  Enhanced Flash Controller                                       
        DCD     FLEXCOM7_Handler                  ; 7  FLEXCOM 7                                        
        DCD     FLEXCOM0_Handler                  ; 8  FLEXCOM 0                                 
        DCD     FLEXCOM1_Handler                  ; 9  FLEXCOM 1                                          
        DCD     0                                 ; 10 Reserved                                              
        DCD     PIOA_Handler                      ; 11 Parallel I/O Controller A                            
        DCD     PIOB_Handler                      ; 12 Parallel I/O Controller B                               
        DCD     PDMIC0_Handler                    ; 13 PDM 0                            
        DCD     FLEXCOM2_Handler                  ; 14 FLEXCOM2                              
        DCD     MEM2MEM_Handler                   ; 15 MEM2MEM                                 
        DCD     I2SC0_Handler                     ; 16 I2SC0                       
        DCD     I2SC1_Handler                     ; 17 I2SC1                            
        DCD     PDMIC1_Handler                    ; 18 PDM 1                      
        DCD     FLEXCOM3_Handler                  ; 19 FLEXCOM3                                       
        DCD     FLEXCOM4_Handler                  ; 20 FLEXCOM4                                      
        DCD     FLEXCOM5_Handler                  ; 21 FLEXCOM5                                         
        DCD     FLEXCOM6_Handler                  ; 22 FLEXCOM6                                     
        DCD     TC0_Handler                       ; 23 Timer/Counter 0                                
        DCD     TC1_Handler                       ; 24 Timer/Counter 1      
        DCD     TC2_Handler                       ; 25 Timer/Counter 2              
        DCD     TC3_Handler                       ; 26 Timer/Counter 3
        DCD     TC4_Handler                       ; 27 Timer/Counter 4                              
        DCD     TC5_Handler                       ; 28 Timer/Counter 5                                           
        DCD     ADC_Handler                       ; 29 Analog To Digital Converter                                   
        DCD     ARM_Handler                       ; 30 FPU  
        DCD     0                                 ; 31 Reserved
        DCD     0                                 ; 32 Reserved
        DCD     0                                 ; 33 Reserved
        DCD     0                                 ; 34 Reserved
        DCD     0                                 ; 35 Reserved
        DCD     0                                 ; 36 Reserved
        DCD     0                                 ; 37 Reserved
        DCD     0                                 ; 38 Reserved
        DCD     0                                 ; 39 Reserved
        DCD     0                                 ; 40 Reserved
        DCD     0                                 ; 41 Reserved
        DCD     0                                 ; 42 Reserved
        DCD     0                                 ; 43 Reserved
        DCD     0                                 ; 44 Reserved
        DCD     0                                 ; 45 Reserved
        DCD     0                                 ; 46 Reserved
        DCD     UHP_Handler                       ; 47 USB OHCI
        DCD     UDP_Handler                       ; 48 USB Device FS
        DCD     CRCCU_Handler                     ; 49 CRCCU 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
Reset_Handler
        LDR     R0, =__iar_program_start
        BX      R0     

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
	 TST LR, #4
	 ITE EQ
	 MRSEQ R0, MSP
	 MRSNE R0, PSP
	 B hard_fault_handler_c

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B vPortSVCHandler
        
        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
DebugMon_Handler
        B DebugMon_Handler
        
        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B xPortPendSVHandler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B xPortSysTickHandler

        PUBWEAK SUPC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SUPC_Handler  
        B SUPC_Handler

        PUBWEAK RSTC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
RSTC_Handler  
        B RSTC_Handler

        PUBWEAK RTC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
RTC_Handler  
        B RTC_Handler

        PUBWEAK RTT_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)  
RTT_Handler  
        B RTT_Handler

        PUBWEAK WDT_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
WDT_Handler  
        B WDT_Handler

        PUBWEAK PMC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PMC_Handler  
        B PMC_Handler

        PUBWEAK EFC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
EFC_Handler  
        B EFC_Handler

        PUBWEAK FLEXCOM7_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
FLEXCOM7_Handler  
        B FLEXCOM7_Handler

        PUBWEAK FLEXCOM0_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
FLEXCOM0_Handler  
        B FLEXCOM0_Handler

        PUBWEAK FLEXCOM1_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
FLEXCOM1_Handler
        B FLEXCOM1_Handler

        PUBWEAK PIOA_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
PIOA_Handler  
        B PIOA_Handler

        PUBWEAK PIOB_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
PIOB_Handler  
        B PIOB_Handler

        PUBWEAK PDMIC0_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
PDMIC0_Handler  
        B PDMIC0_Handler

        PUBWEAK FLEXCOM2_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
FLEXCOM2_Handler  
        B FLEXCOM2_Handler

        PUBWEAK MEM2MEM_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
MEM2MEM_Handler  
        B MEM2MEM_Handler

        PUBWEAK I2SC0_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
I2SC0_Handler  
        B I2SC0_Handler

        PUBWEAK I2SC1_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
I2SC1_Handler  
        B I2SC1_Handler

        PUBWEAK PDMIC1_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
PDMIC1_Handler  
        B PDMIC1_Handler

        PUBWEAK FLEXCOM3_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
FLEXCOM3_Handler  
        B FLEXCOM3_Handler

        PUBWEAK FLEXCOM4_Handler
        SECTION .text:CODE:REORDER:NOROOT(1) 
FLEXCOM4_Handler  
        B FLEXCOM4_Handler

        PUBWEAK FLEXCOM5_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)  
FLEXCOM5_Handler  
        B FLEXCOM5_Handler

        PUBWEAK FLEXCOM6_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)  
FLEXCOM6_Handler  
        B FLEXCOM6_Handler

        PUBWEAK TC0_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)  
TC0_Handler  
        B TC0_Handler

        PUBWEAK TC1_Handler
        SECTION .text:CODE:REORDER:NOROOT(1) 
TC1_Handler
        B TC1_Handler

        PUBWEAK TC2_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
TC2_Handler  
        B TC2_Handler

        PUBWEAK TC3_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
TC3_Handler  
        B TC3_Handler

        PUBWEAK TC4_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
TC4_Handler  
        B TC4_Handler
        
        PUBWEAK TC5_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)    
TC5_Handler  
        B TC5_Handler

        PUBWEAK ADC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
ADC_Handler  
        B ADC_Handler

        PUBWEAK ARM_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
ARM_Handler  
        B ARM_Handler

        PUBWEAK UHP_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UHP_Handler  
        B UHP_Handler

        PUBWEAK UDP_Handler
        SECTION .text:CODE:REORDER:NOROOT(1) 
UDP_Handler  
        B UDP_Handler

        PUBWEAK CRCCU_Handler
        SECTION .text:CODE:REORDER:NOROOT(1) 
CRCCU_Handler  
        B CRCCU_Handler

        END
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
