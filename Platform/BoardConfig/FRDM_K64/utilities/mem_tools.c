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

/*******************************************************************************
 * Application Included Files
 ******************************************************************************/
#include "mem_tools.h"
/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
void *mem_align(size_t ptrSize, uint32_t alignment)
{
    char gap;   /* Variable to store difference between memory locations. */

    /* Allocate more than enough memory. */
    char *temp = (char *)OSA_MemAlloc(ptrSize + (alignment - 1));
    if(temp == NULL)
    {
        return NULL;  /* Return NULL if not enough space for allocation request. */
    }

    /* Create a pointer that is aligned correctly. */
    char *ptr = (char *)(((uint32_t)temp + (alignment - 1)) & ~(uint32_t)(alignment - 1));

    /* Calculate the gap between start of allocation and aligned pointer. */
    gap = (char)((unsigned int)ptr - (unsigned int)temp);

    if(gap)
    {
        ptr--;                /* Decrement the pointer. */
        *ptr = gap;           /* Store gap size. */
        ptr++;                /* Return the pointer to aligned address. */
        return (void *)ptr;
    } /* If there exists a gap between allocated memory and aligned pointer. */
    else
    {
        return (void *)ptr;
    }/* If no gap. */

}

void free_align(void *ptr)
{
    char pad;
    char count;

    if(ptr != NULL)
    {
        char *temp = ptr;
        temp--;        /* Decrement pointer to find the padding size. */
        pad = *temp;
        temp++;
        count = 0;
        while(count < pad)  /* Decrement pointer to start of previously allocated memory. */
        {
            temp--;
            count++;
        }
        OSA_MemFree(temp);
        temp = NULL;
    }

}

/******************************************************************************
 * EOF
 ******************************************************************************/
