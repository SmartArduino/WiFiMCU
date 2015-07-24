/*****************************************************************************
 * (c) Copyright 2011, Freescale Semiconductor Inc.
 * ALL RIGHTS RESERVED.
 ***************************************************************************//*!
 * @file      flash_kinetis.h
 * @author    b45112
 * @version   1.0.8.0
 * @date      Dec-02-2014
 * @brief     Flash programming driver header file
 * @par       
 * @include   
 * @par       
 * @include         
 ******************************************************************************/
#ifndef _FLASH_KINETIS_H
#define _FLASH_KINETIS_H

#include "board.h"

typedef unsigned char Byte;
typedef unsigned long LWord;
typedef unsigned short Word;

//  Flash hardware algorithm operation commands 
#define FLASH_PROGRAM_LONGWORD    0x06
#define FLASH_PROGRAM_PHRASE      0x07
#define FLASH_ERASE_SECTOR        0x09



#define FLASH_INIT_FLASH_CLOCK SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV4(1);
#define FLASH_BASE_PTR                FTFE_BASE_PTR
#define FLASH_FSTAT                   FTFE_FSTAT                                  
#define FLASH_FSTAT_CCIF_MASK         FTFE_FSTAT_CCIF_MASK
#define FLASH_FSTAT_ACCERR_MASK       FTFE_FSTAT_ACCERR_MASK
#define FLASH_FSTAT_FPVIOL_MASK       FTFE_FSTAT_FPVIOL_MASK
#define FLASH_FSTAT_RDCOLERR_MASK     FTFE_FSTAT_RDCOLERR_MASK
#define FLASH_FSTAT_MGSTAT0_MASK      FTFE_FSTAT_MGSTAT0_MASK 

#define FLASH_PROGRAM                 FLASH_ProgramSectionByPhrases



#define FCCOB_REGS  12
#define FLASH_OK     0
#define FLASH_FAIL   1

#define FLASH_BURST_RAM_ADDR (LWord*)0x14000000
#define FLASH_BURST_RAM_SIZE	64


#define ERASE_BLOCK_INDEX       4
#define PROGRAM_LONGWORD_INDEX  8
#define PROGRAM_PHRASE_INDEX    12

//  FCOOB register structure
typedef union 
{
  Byte all[FCCOB_REGS];
  struct
  {
    Byte fccob3;
    Byte fccob2;
    Byte fccob1;
    Byte fccob0;
    Byte fccob7;
    Byte fccob6;
    Byte fccob5;
    Byte fccob4;
    Byte fccobB;
    Byte fccobA;
    Byte fccob9;
    Byte fccob8;
  }regs;
  
  struct
  {
    LWord fccob3210;
    LWord fccob7654;
    LWord fccobBA98;
  }regsLong;
}FCC0B_STR;

//  API FUNCTION FOR KINETIS FLASH DRIVER
void  FLASH_Initialization(void);
LWord FLASH_EraseSector(LWord destination);
LWord FLASH_ProgramPhrase(LWord destination, LWord * data64b);
LWord FLASH_ProgramSectionByPhrases(LWord destination, LWord* pSource, LWord size);


#endif



