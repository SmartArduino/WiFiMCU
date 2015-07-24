/**
******************************************************************************
* @file    hardfault.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide debug information in hardfault.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 
#include "board.h"

struct exception_stack_frame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
};


void hard_fault_handler_c (struct exception_stack_frame* contex)
{
	DEBUGOUT("psr: 0x%08x\n", contex->psr);
	DEBUGOUT(" pc: 0x%08x\n", contex->pc);
	DEBUGOUT(" lr: 0x%08x\n", contex->lr);
	DEBUGOUT("r12: 0x%08x\n", contex->r12);
	DEBUGOUT("r03: 0x%08x\n", contex->r3);
	DEBUGOUT("r02: 0x%08x\n", contex->r2);
	DEBUGOUT("r01: 0x%08x\n", contex->r1);
	DEBUGOUT("r00: 0x%08x\n", contex->r0);  
 
  while (1);
}