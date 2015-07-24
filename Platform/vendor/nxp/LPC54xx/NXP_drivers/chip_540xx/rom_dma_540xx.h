/*
%
% Copyright 2012 NXP Semiconductors,
% 411 E Plumeria Dr San Jose CA USA
% All rights are reserved. Reproduction in whole or in part is prohibited
% without the prior written consent of the copyright owner.
%
*/

/*
+++ IDENTIFICATION

PACKAGE           : 
COMPONENT         : 
INTERFACE         : 
TEMPLATE VERSION  : 3
*/

#ifndef __ROM_DMA_412X_H
#define __ROM_DMA_412X_H

//Macro for parameter
//for event
#define DMA_SW_REQ         0//software request
#define DMA_PER_REQ        1//peripheral request
#define DMA_HD_REQ         2//hardware request
//for source burst
#define DMA_SRC_WRAP       (1<<4) //source burst wrapping is enabled
//for destination burst
#define DMA_DST_WRAP       (1<<5) //destination burst wrapping is enabled
//for Ping_Pong mode
#define DMA_PING_PONG      1//Linked with previous task for Ping_Pong transfer
//for software trigger
#define DMA_SW_ON          (1<<1) //software trigger is set immediately for this task
//for clear trigger
#define DMA_CLEAR_TRIGGER  (1<<2) //clear trigger when task is finished
//for INT A flag
#define DMA_INT_A          (1<<3) //set Interrupt flag A when task is finished
//for INT B flag
#define DMA_INT_B          (1<<4) //set Interrupt flag B when task is finished
//for data width
#define DMA_8_BIT          0//the data width is 8-bit
#define DMA_16_BIT         1//the data width is 16-bit
#define DMA_32_BIT         2//the data width is 32-bit
//for source data increment
#define DMA_SRC_INC_1          (1<<2) //The source address is incremented by 1 data width
#define DMA_SRC_INC_2          (2<<2) //The source address is incremented by 2 data width
#define DMA_SRC_INC_4          (3<<2) //The source address is incremented by 4 data width
//for destination data increment
#define DMA_DST_INC_1          (1<<4) //The destination address is incremented by 1 data width
#define DMA_DST_INC_2          (2<<4) //The destination address is incremented by 2 data width
#define DMA_DST_INC_4          (3<<4) //The destination address is incremented by 4 data width

typedef void   DMA_HANDLE_T ;    // define TYPE for DMA handle pointe
typedef void   (*CALLBK_T) (uint32_t err_code, uint32_t n); //define callback func TYPE

typedef struct  DMA_CHANNEL {
  uint8_t event;       // event type selection for DMA transfer
                          //0: software request
                          //1: peripheral request
                          //2: hardware trigger
                          //others: reserved
  uint8_t hd_trigger;  //In case hardware trigger is enabled, the trigger burst is setup here.
                       //NOTE: Rising edge triggered is fixed
                       //bit0~bit3: burst size
                          //0: burst size =1, 1: 2^1, 2: 2^2,... 10: 1024, others: reserved.
                       //bit4: Source Burst Wrap
                          //0: Source burst wrapping is not enabled
                          //1: Source burst wrapping is enabled
                       //bit5: Destination Burst Wrap
                          //0: Destination burst wrapping is not enabled
                          //1: Destination burst wrapping is enabled
                       //bit6: Trigger Burst
                          //0: Hardware trigger cause a single transfer
                          //1: Hardware trigger cause a burst transfer
                       //bit7: reserved
  uint8_t priority;    //priority level
                          //0 -> 7: Highest priority ->  Lowest priority.
                          //other: reserved.
  CALLBK_T callback_func_pt; 	// callback function, Callback function is 
                                      // only invoked when INTA or INTB is enabled,
}  DMA_CHANNEL_T  ;

typedef struct  DMA_TASK {
  uint8_t ch_num;     // DMA channel number
  uint8_t config;      //configuration of this task
                       //bit0: Ping_Pong transfer
                          //0: Not Ping_Pong transfer
                          //1: Linked with previous task for Ping_Pong transfer
                       //bit1: Software Trigger.
                          //0: the trigger for this channel is not set.
                          //1: the trigger for this channel is set immediately.
                       //bit2:  Clear Trigger
                          //0: The trigger is not cleared when this task is finished.
                          //1: The trigger is cleared when this task is finished.
                       //bit3:  Select INTA
                          //0: No IntA.
                          //1: The IntB flag for this channel will be set when this task is finished.
                       //bit4:  Select INTB
                          //0: No IntB.
                          //1: The IntB flag for this channel will be set when this task is finished.
                       //bit5~bit7: reserved

  uint8_t data_type;
                       //bit0~bit1: Data width. 0: 8-bit, 1: 16-bit, 2: 32-bit, 3: reserved
                       //bit2~bit3: How is source address incremented?
                               //0: The source address is not incremented for each transfer.
                               //1: The source address is incremented by the amount specified by Width for each transfer.
                               //2: The source address is incremented by 2 times the amount specified by Width for each transfer.
                               //3: The source address is incremented by 4 times the amount specified by Width for each transfer.
                       //bit4~bit5: How is the destination address incremented?
                               //0: The destination address is not incremented for each transfer.
                               //1: The destination address is incremented by the amount specified by Width for each transfer.
                               //2: The destination address is incremented by 2 times the amount specified by Width for each transfer.
                               //3: The destination address is incremented by 4 times the amount specified by Width for each transfer.
                       //bit6~bit7: reserved.
  uint16_t data_length;  //0: 1 transfer, 1: 2 transfer, ..., 1023: 1024 transfer. Others: reserved.
  uint32_t src;          // Source data end address
  uint32_t dst;          // Destination end address
  uint32_t task_addr;    //the address of RAM for saving this task.
                         //(NOTE: each task need 16 bytes RAM for storing configuration, 
                         //  and DMA API could set it according user input parameter, 
                         //  but it is responsible of user to allocate this RAM space and 
                         //  make sure that the base address must be 16-byte alignment. 
                         //  And if user has setup the next_tast(!=0), the dma_task_link 
                         //  must be called for this task setup, otherwise unpredictable error will happen.)
}  DMA_TASK_T ;


//************************************************
// *** Below is the structure providing the function types for all the
//     DMA functions.  One structure is used to pass params to most of the functions.  
//     The parms that are required to be set before calling the function are 
//     listed in comments below each function.	***
 
typedef struct  DMAD_API {	   // index of all the DMA driver functions
  void          (*dma_isr)(DMA_HANDLE_T*  handle);
  uint32_t      (*dma_get_mem_size)( void);
  DMA_HANDLE_T* (*dma_setup)( uint32_t base_addr,  uint8_t *ram );
  ErrorCode_t   (*dma_init)( DMA_HANDLE_T*  handle, DMA_CHANNEL_T *channel, DMA_TASK_T *task);
  ErrorCode_t   (*dma_link)( DMA_HANDLE_T* handle, DMA_TASK_T *task, uint8_t valid);
  ErrorCode_t   (*dma_set_valid)( DMA_HANDLE_T* handle, uint8_t chl_num);
  ErrorCode_t   (*dma_pause)( DMA_HANDLE_T* handle, uint8_t chl_num);
  ErrorCode_t   (*dma_unpause)( DMA_HANDLE_T* handle, uint8_t chl_num);
  ErrorCode_t   (*dma_abort)( DMA_HANDLE_T* handle, uint8_t chl_num);
  uint32_t      (*dma_get_firmware_version)( void );
} DMAD_API_T  ;	// end of structure ************************************

  
extern const DMAD_API_T  dma_api ;  //so application program can access	pointer to
                                    // function table

#endif /* __ROM_DMA_412X_H */
