/*****************************************************************************
 * (c) Copyright 2011, Freescale Semiconductor Inc.
 * ALL RIGHTS RESERVED.
 ***************************************************************************//*!
 * @file      flash_kinetisRamFunc.c
 * @author    b45112
 * @version   1.0.1.0
 * @date      Dec-02-2014
 * @brief     Flash programming driver
 * @par       
 * @include   
 * @par       
 * @include         
 ******************************************************************************/

#include "flash_kinetis.h"

extern FCC0B_STR CommandObj;
  
//  internal driver function
/********************************************************
* Function for executing of the flash command
*
********************************************************/
LWord FLASH_FlashCommandSequenceStart(Byte index)
{
  Byte* ptrFccobReg = (Byte*)&FLASH_BASE_PTR->FCCOB3;
  Byte* ptrCommandObj = (Byte*)&CommandObj;

  
  /* wait till CCIF bit is set */
  while(!(FLASH_FSTAT & FLASH_FSTAT_CCIF_MASK)){};
  /* clear RDCOLERR & ACCERR & FPVIOL flag in flash status register */
  FLASH_FSTAT = FLASH_FSTAT_ACCERR_MASK | FLASH_FSTAT_FPVIOL_MASK | FLASH_FSTAT_RDCOLERR_MASK;  
  
  /* load FCCOB registers */  
  while(index--)
    *ptrFccobReg++ = *ptrCommandObj++;
  
  //  launch a command 
  FLASH_FSTAT |= FLASH_FSTAT_CCIF_MASK; 
  //  waiting for the finishing of the command
  while(!(FLASH_FSTAT & FLASH_FSTAT_CCIF_MASK)){};

  
   /* Check error bits */
  /* Get flash status register value */
  return (FLASH_FSTAT & (FLASH_FSTAT_ACCERR_MASK | FLASH_FSTAT_FPVIOL_MASK | FLASH_FSTAT_MGSTAT0_MASK));  
} 



