/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
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
/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ffconf.h"     /* FatFs configuration options */
#include "diskio.h"     /* FatFs lower layer API */
#include "malloc.h"		
#include "usdhc1_sdcard.h"
#include "w25qxx.h"
#include "malloc.h"	
#include "fsl_sd.h"


#define SD_CARD	 	    0  			//SD卡,卷标为0
#define EX_FLASH        2           //外部spi flash,卷标为2


//对于W25Q256
//前25M字节给fatfs用,25M字节后,用于存放字库,字库占用6.01M.	剩余部分,给客户自己用	 
#define FATFS_FLASH_SECTOR_SIZE 	512	
#define FATFS_FLASH_SECTOR_COUNT 	1024*25*2	//W25Q256,前25M字节给FATFS占用	
#define FATFS_FLASH_BLOCK_SIZE   	8     		//每个BLOCK有8个扇区		

extern sd_card_t usdhc1_sd;                    //USDHC1 SD卡


DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS res=STA_NOINIT;
    switch (pdrv)
    {
        case SD_CARD:
            res=RES_OK;
            break;
        case EX_FLASH:
            res=RES_OK;
            break;
        default:
            break;
    }
    return res;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS res=STA_NOINIT;
    switch (pdrv)
    {
        case SD_CARD:
            res=USDHC1_Init();	//SD卡初始化 
            break;
        case EX_FLASH:
            W25QXX_Init();  //W25QXX初始化
            res=RES_OK;
            break;
        default:
            break;
    }
    return res;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res=RES_PARERR;
    switch (pdrv)
    {
        case SD_CARD:       //SD卡
            res=(DRESULT)SD_ReadDisk(buff,sector,count);	 
			while(res)//读出错
			{
				USDHC1_Init();	//重新初始化SD卡
				res=(DRESULT)SD_ReadDisk(buff,sector,count);	
				//printf("sd rd error:%d\r\n",res);
			}
            break;  
        case EX_FLASH://外部flash
			for(;count>0;count--)
			{
				W25QXX_Read(buff,sector*FATFS_FLASH_SECTOR_SIZE,FATFS_FLASH_SECTOR_SIZE);
				sector++;
				buff+=FATFS_FLASH_SECTOR_SIZE;
			}
			res=RES_OK;
			break;
        default:
            break;
    }

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res=RES_PARERR;
    switch (pdrv)
    {
        case SD_CARD://SD卡
			res=(DRESULT)SD_WriteDisk((u8*)buff,sector,count);
			while(res)//写出错
			{
				USDHC1_Init();	//重新初始化SD卡
				res=(DRESULT)SD_WriteDisk((u8*)buff,sector,count);	
				//printf("sd wr error:%d\r\n",res);
			}
			break;
        case EX_FLASH://外部flash
			for(;count>0;count--)
			{										    
				W25QXX_Write((u8*)buff,sector*FATFS_FLASH_SECTOR_SIZE,FATFS_FLASH_SECTOR_SIZE);
				sector++;
				buff+=FATFS_FLASH_SECTOR_SIZE;
			}
			res=RES_OK;
			break;
        default:
            break;
    }
    return res;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res=RES_PARERR;
    
    if(pdrv==SD_CARD)//SD卡
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
				*(DWORD*)buff = usdhc1_sd.blockSize; 
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE: 
				*(WORD*)buff = usdhc1_sd.csd.eraseSectorSize;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = usdhc1_sd.blockCount;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}else if(pdrv==EX_FLASH)	//外部FLASH  
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = FATFS_FLASH_SECTOR_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = FATFS_FLASH_BLOCK_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = FATFS_FLASH_SECTOR_COUNT;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
    return res;
}
