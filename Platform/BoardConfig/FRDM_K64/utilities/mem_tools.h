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

/*
    mem_tools.h

    @brief  memory allocation functions
    @func   mem_align Returns pointer to aligned dynamically allocated memory (similar to memalign() in GCC)
 */


#ifndef __MEM_TOOLS_H__
#define __MEM_TOOLS_H__
/*******************************************************************************
 * Standard C Included Files
 ******************************************************************************/
#include <stdlib.h>
#include <stdint.h>

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*
    @brief  Function  to peform aligned data memory allocation. Useful when memalign is not available.
    @param  ptrSize   size_t variable to pass size of memory to be allocated
    @param  alignment uint32_t variable to pass byte size to align data with
    @return pointer to aligned & allocated memory
 */
void *mem_align(size_t ptrSize, uint32_t alignment);

/*
    @brief  Function  to free memory allocated by mem_align
    @param  ptr  pointer that has been allocated with mem_align
 */
void free_align(void *ptr);


#endif /* __MEM_TOOLS_H__ */

/******************************************************************************
 * EOF
 ******************************************************************************/
