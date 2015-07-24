#include "diskio.h"

#include "MicoDriverFlash.h "
#include "MicoDriverWdg.h"
#include "Common.h "
#include "lua.h"

/*
//Flash相关参数
#define FLASH_PAGE_SIZE			256
#define FLASH_SECTOR_SIZE		4096
#define FLASH_SECTOR_COUNT		512
#define FLASH_BLOCK_SIZE		65536
#define FLASH_PAGES_PER_SECTOR	FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE
*/

#define MICO_FLASH_FOR_LUA       MICO_SPI_FLASH  /* Optional */
#define LUA_START_ADDRESS        (uint32_t)0x00100000 /* Optional */
#define LUA_END_ADDRESS          (uint32_t)0x001FFFFF /* Optional */
#define LUA_FLASH_SIZE           (LUA_END_ADDRESS - LUA_START_ADDRESS + 1) /* 512k bytes, optional*/

/*
#define BLOCK_SIZE      4096
#define SECTOR_SIZE     2048
#define SECTOR_COUNT    4096
*/
#define BLOCK_SIZE      65536
#define SECTOR_SIZE     256
#define SECTOR_COUNT    1024


// Inidialize a Drive
DSTATUS disk_initialize(BYTE drv)		/* Physical drive nmuber(0..) */
{
	
    MicoFlashInitialize(MICO_FLASH_FOR_LUA);
    MCU_DBG("\r\n MicoFlashInitialize is called\r\n");
    return RES_OK;
}



DSTATUS disk_status(BYTE drv)			/* Physical drive nmuber(0..) */
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read(
	BYTE drv,		/* Physical drive nmuber(0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address(LBA) */
	BYTE count		/* Number of sectors to read(1..255) */
)
{	
	/*int i;
	for(i=0;i<count;i++)
	{
		W25X_Read_Sector(sector,buff);
		sector ++;
		buff += FLASH_SECTOR_SIZE;
	}
	
	return RES_OK;*/
  
    uint32_t BufferSize = (uint32_t)(BLOCK_SIZE * count); 
    uint32_t Address = (uint32_t ) (LUA_START_ADDRESS + (sector * BLOCK_SIZE)); 
  
    MCU_DBG("disk_read is called,Address:%d,BufferSize:%d,BLOCK_SIZE:%d,sector:%d\r\n",Address,BufferSize,BLOCK_SIZE,count,sector);
    
    if((Address+BufferSize)>(uint32_t )LUA_END_ADDRESS) 
      return RES_ERROR;
  
    //MCU_DBG("\r\n Address:%ld,buff:%s,buffersize:%ld\r\n",Address,buff,BufferSize);
    MicoFlashRead(MICO_FLASH_FOR_LUA,&Address,(uint8_t*)buff,BufferSize);
    
    for(int i=0;i<10;i++)
      MCU_DBG("readdata: i:%d,data:%d\r\n",i,buff[i]);
      
    return RES_OK;
        
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#include "spi_flash.h"
extern OSStatus spiFlashErase(uint32_t StartAddress, uint32_t EndAddress);
extern sflash_handle_t sflash_handle;

#if _READONLY == 0
DRESULT disk_write(
	BYTE drv,			/* Physical drive nmuber(0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address(LBA) */
	BYTE count			/* Number of sectors to write(1..255) */
)
{
	/*int i;
	for(i=0;i<count;i++)
	{	W25X_Erase_Sector(sector);
	
		W25X_Write_Sector(sector,(u8*)buff);
		sector ++;
		buff += FLASH_SECTOR_SIZE;
	}
	
	return RES_OK;	*/
  
    MCU_DBG("\r\n disk_write is called\r\n");
    
    uint32_t BufferSize = (uint32_t)(BLOCK_SIZE * count); 
    uint32_t Address = (uint32_t ) (LUA_START_ADDRESS + (sector * BLOCK_SIZE)); 
    uint32_t endAddress = Address+BufferSize-1;
    
    if((Address+BufferSize)>(uint32_t )LUA_END_ADDRESS) return RES_ERROR;
    
    //MCU_DBG("\r\n MicoFlashInitialize is called\r\n");
    //MicoFlashInitialize((mico_flash_t)MICO_FLASH_FOR_LUA);
    //MCU_DBG("MicoFlashInitialize:id:%x,%d\r\n",sflash_handle.device_id,sflash_handle.write_allowed);
    
    MCU_DBG("\r\n MicoFlashErase is called: Address:%x,endAddress:%x,%ld\r\n",Address,endAddress,BufferSize);
    spiFlashErase(Address,endAddress);
    
    Address = (uint32_t ) (LUA_START_ADDRESS + (sector * BLOCK_SIZE)); 
    
    MCU_DBG("\r\n sflash_write is called: Address:%x,%ld\r\n",Address,BufferSize);
    MicoFlashWrite( MICO_FLASH_FOR_LUA, &Address, (uint8_t*)buff, BufferSize);
    //sflash_write( &sflash_handle, Address, (uint8_t*)buff, BufferSize );
    
    //MCU_DBG("\r\n MicoFlashFinalize is called: Address:%x,%ld,%s\r\n",Address,BufferSize,buff);
    //MicoFlashFinalize(MICO_FLASH_FOR_LUA);
    
    MicoWdgReload();
    
   for(int i=0;i<10;i++)
     MCU_DBG("wirtedata: i:%d,data:%d\r\n",i,buff[i]);

    return RES_OK;
}
#endif /* _READONLY */




/************************************************************************************
CTRL_SYNC			Make sure that the disk drive has finished pending write process. 
					When the disk I/O module has a write back cache, flush the dirty sector immediately. 
					This command is not used in read-only configuration. 
GET_SECTOR_SIZE		Returns sector size of the drive into the WORD variable pointed by Buffer. 
					This command is not used in fixed sector size configuration, _MAX_SS is 512. 
GET_SECTOR_COUNT	Returns number of available sectors on the drive into the DWORD variable pointed by Buffer.
					This command is used by only f_mkfs function to determine the volume size to be created. 
GET_BLOCK_SIZE		Returns erase block size of the flash memory in unit of sector into the DWORD variable pointed by Buffer.
					The allowable value is 1 to 32768 in power of 2. 
					Return 1 if the erase block size is unknown or disk devices. 
					This command is used by only f_mkfs function and it attempts to align data area to the erase block boundary. 
CTRL_ERASE_SECTOR	Erases a part of the flash memory specified by a DWORD array {<start sector>, <end sector>} pointed by Buffer. 
					When this feature is not supported or not a flash memory media, this command has no effect.
					The FatFs does not check the result code and the file function is not affected even if the sectors are not erased well. 
					This command is called on removing a cluster chain when _USE_ERASE is 1. 
************************************************************************************/
DRESULT disk_ioctl(
	BYTE drv,		/* Physical drive nmuber(0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_OK;
	//DWORD nFrom,nTo;
//	int i;
//	char *buf = buff;
	MCU_DBG("\r\n disk_ioctl is called\r\n");
        
	switch(ctrl)
	{
		case CTRL_SYNC :
			break;
		
		//扇区擦除
		case CTRL_ERASE_SECTOR:
			/*
                        nFrom = *((DWORD*)buff);
			nTo = *(((DWORD*)buff)+1);
			for(i=nFrom;i<=nTo;i++)
				W25X_Erase_Sector(i);
			*/	
                  MCU_DBG("\r\n disk_ioctl CTRL_ERASE_SECTOR is called\r\n");
			break;
		
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = BLOCK_SIZE;
                        MCU_DBG("\r\n disk_ioctl GET_BLOCK_SIZE is called\r\n");
			break;
	
	
		case GET_SECTOR_SIZE:
			*(DWORD*)buff = SECTOR_SIZE;
                        MCU_DBG("\r\n disk_ioctl GET_SECTOR_SIZE is called\r\n");
			break;
		
		case GET_SECTOR_COUNT:
			*(DWORD*)buff = SECTOR_COUNT;
                        MCU_DBG("\r\n disk_ioctl GET_SECTOR_COUNT is called\r\n");
			break;
			
		default:
			res = RES_PARERR;
			break;
	}
	return res;
}

DWORD get_fattime(void)
{
/*
 t = Time_GetCalendarTime();
    t.tm_year -= 1980;  //年份改为1980年起
    t.tm_mon++;          //0-11月改为1-12月
    t.tm_sec /= 2;       //将秒数改为0-29
    
    date = 0;
    date = (t.tm_year << 25) | (t.tm_mon<<21) | (t.tm_mday<<16)|\
            (t.tm_hour<<11) | (t.tm_min<<5) | (t.tm_sec);

    return date;
*/
	return 0;
}


