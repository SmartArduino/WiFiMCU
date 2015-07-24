/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _USER_CFG_H_
#define _USER_CFG_H_

/* size of array to copy__Launch_Command function to.*/
/* It should be at least equal to actual size of __Launch_Command func */
/* User can change this value based on RAM size availability and actual size of __Launch_Command function */
#define LAUNCH_CMD_SIZE         0x100

/* Size of function used for callback.  Change this depending on the size of your function */
#define CALLBACK_SIZE           0x80

/***********************************************************************************************************
* Derivative selection.  Use this to select correct header file
* Depending upon the macro selected, There will be a change in some internal macros and source codes.
* The derivative name will be defined based on the following rule:
*
*       FTFx_AA_BB_CC_DD_EE_FF
*
*               In which:
*               FTFx: abbreviation for C90TFS/FTFx flash family.
*               AA: MCU type.
*               BB: P-Flash block size.
*               CC: FlexNVM block size.
*               DD: FlexRam or Acceleration Ram size.
*               EE: P-Flash sector size.
*               FF: D-Flash sector size.
****************************************************************************************************************/
#if (defined(CPU_MK64FN1M0VMD12))
/* 1024K P-Flash, 0K FlexNVM, 4K FlexRam, 4K P-Flash sector size, 0K D-Flash sector size */
#define FLASH_DERIVATIVE                  FTFx_KX_1024K_0K_4K_4K_0K
#elif (defined(CPU_MK22FN512VDC12) || defined(CPU_MKV31F512VLL12) || defined(CPU_MK22FN512VLH12))
/* 512K P-Flash, 0K FlexNVM, 0K FlexRam, 2K P-Flash sector size, 0K D-Flash sector size */
#define FLASH_DERIVATIVE                  FTFx_KX_512K_0K_0K_2K_0K
#elif (defined(CPU_MK22FN256VDC12) || defined(CPU_MKV31F256VLL12))
/* 256K P-Flash, 0K FlexNVM, 0K FlexRam, 2K P-Flash sector size, 0K D-Flash sector size */
#define FLASH_DERIVATIVE                  FTFx_KX_256K_0K_0K_2K_0K
#elif (defined(CPU_MK22FN128VDC10) || defined(CPU_MKV31F128VLL10))
/* 128K P-Flash, 0K FlexNVM, 0K FlexRam, 2K P-Flash sector size, 0K D-Flash sector size */
#define FLASH_DERIVATIVE                  FTFx_KX_128K_0K_0K_2K_0K
#endif
#endif /* End of _USER_CFG_H_ */





