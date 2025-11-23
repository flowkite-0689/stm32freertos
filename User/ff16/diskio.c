/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */
#include "spi.h"		/* SPI and W25Q128 functions */


/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH	0	/* Map W25Q128 to physical drive 0 */
#define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Map USB MSD to physical drive 2 */

/* Disk Status */
static volatile DSTATUS Stat = STA_NOINIT;	/* Physical drive status */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case DEV_FLASH :
		stat = Stat;
		return stat;

	case DEV_MMC :
		// result = MMC_disk_status();
		// translate the reslut code here
		return STA_NOINIT;

	case DEV_USB :
		// result = USB_disk_status();
		// translate the reslut code here
		return STA_NOINIT;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	unsigned long jedec_id;

	switch (pdrv) {
	case DEV_FLASH :
		// Initialize SPI for W25Q128
		SPI1_Init();
		
		// Check JEDEC ID of W25Q128
		jedec_id = W25Q128_ReadID();
		if (jedec_id == 0xEF4018) {
			stat = 0;  // Successfully initialized
		} else {
			stat = STA_NOINIT;  // Failed to initialize
		}
		
		Stat = stat;
		return stat;

	case DEV_MMC :
		// result = MMC_disk_initialize();
		// translate the reslut code here
		return STA_NOINIT;

	case DEV_USB :
		// result = USB_disk_initialize();
		// translate the reslut code here
		return STA_NOINIT;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	UINT bytes_to_read;

	switch (pdrv) {
	case DEV_FLASH :
		// Convert sector to byte address (assuming 512-byte sectors)
		bytes_to_read = count * 512;
		W25Q128_ReadData(buff, sector * 512, bytes_to_read);
		res = RES_OK;
		return res;

	case DEV_MMC :
		// translate the arguments here
		// result = MMC_disk_read(buff, sector, count);
		// translate the reslut code here
		return RES_PARERR;

	case DEV_USB :
		// translate the arguments here
		// result = USB_disk_read(buff, sector, count);
		// translate the reslut code here
		return RES_PARERR;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	UINT bytes_to_write;
	UINT i;
	unsigned char result;

	switch (pdrv) {
	case DEV_FLASH :
		// Convert sector to byte address (assuming 512-byte sectors)
		bytes_to_write = count * 512;
		
		// Erase sectors before writing
		for (i = 0; i < count; i++) {
			result = W25Q128_SectorErase((sector + i) * 512);
			if (result != W25Q128_RESULT_OK) {
				return RES_ERROR;
			}
		}
		
		// Write data
		result = W25Q128_BufferWrite((unsigned char *)buff, sector * 512, bytes_to_write);
		if (result != W25Q128_RESULT_OK) {
			return RES_ERROR;
		}
		
		res = RES_OK;
		return res;

	case DEV_MMC :
		// translate the arguments here
		// result = MMC_disk_write(buff, sector, count);
		// translate the reslut code here
		return RES_PARERR;

	case DEV_USB :
		// translate the arguments here
		// result = USB_disk_write(buff, sector, count);
		// translate the reslut code here
		return RES_PARERR;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	switch (pdrv) {
	case DEV_FLASH :
		switch (cmd) {
		case CTRL_SYNC:
			// Make sure that no pending write process
			// All write operations are blocking in our implementation
			res = RES_OK;
			break;

		case GET_SECTOR_COUNT:
			// Get number of sectors on the disk (WORD)
			*(LBA_t*)buff = W25Q128_CAPACITY / 512;  // Total sector count
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE:
			// Get sector size (WORD)
			*(WORD*)buff = 512;  // Sector size is 512 bytes
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE:
			// Get erase block size in unit of sector (DWORD)
			*(DWORD*)buff = W25Q128_SECTOR_SIZE / 512;  // Block size in sectors
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
		}
		return res;

	case DEV_MMC :
		// Process of the command for the MMC/SD card
		return RES_PARERR;

	case DEV_USB :
		// Process of the command the USB drive
		return RES_PARERR;
	}

	return RES_PARERR;
}
