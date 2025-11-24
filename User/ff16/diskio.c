/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h" /* Declarations FatFs MAI */
#include "spi.h"		/* SPI and W25Q128 functions */

/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH 0 /* Map W25Q128 to physical drive 0 */
#define DEV_MMC 1		/* Map MMC/SD card to physical drive 1 */
#define DEV_USB 2		/* Map USB MSD to physical drive 2 */

/* Disk Status */
static volatile DSTATUS Stat = STA_NOINIT; /* Physical drive status */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
		BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv)
	{
	case DEV_FLASH:
		stat = Stat;
		return stat;

	case DEV_MMC:
		// result = MMC_disk_status();
		// translate the reslut code here
		return STA_NOINIT;

	case DEV_USB:
		// result = USB_disk_status();
		// translate the reslut code here
		return STA_NOINIT;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
		BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	unsigned long jedec_id;

	switch (pdrv)
	{
	case DEV_FLASH:
		// Initialize SPI for W25Q128
		SPI1_Init();

		// Check JEDEC ID of W25Q128
		jedec_id = W25Q128_ReadID();
		if (jedec_id == 0xEF4018)
		{
			stat = 0; // Successfully initialized
		}
		else
		{
			stat = STA_NOINIT; // Failed to initialize
		}

		Stat = stat;
		return stat;

	case DEV_MMC:
		// result = MMC_disk_initialize();
		// translate the reslut code here
		return STA_NOINIT;

	case DEV_USB:
		// result = USB_disk_initialize();
		// translate the reslut code here
		return STA_NOINIT;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
		BYTE pdrv,		/* Physical drive nmuber to identify the drive */
		BYTE *buff,		/* Data buffer to store read data */
		LBA_t sector, /* Start sector in LBA */
		UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	UINT bytes_to_read;

	switch (pdrv)
	{
	case DEV_FLASH:
		// Convert sector to byte address (assuming 512-byte sectors)
		bytes_to_read = count * 512;
		W25Q128_ReadData(buff, sector * 512, bytes_to_read);
		res = RES_OK;
		return res;

	case DEV_MMC:
		// translate the arguments here
		// result = MMC_disk_read(buff, sector, count);
		// translate the reslut code here
		return RES_PARERR;

	case DEV_USB:
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

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != DEV_FLASH) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    uint32_t start_addr = sector * 512;
    uint32_t end_addr = start_addr + count * 512;
    uint32_t erase_start = start_addr & 0xFFFFF000;  // 4KB 对齐
    uint32_t erase_end = (end_addr + 4095) & 0xFFFFF000;

    // printf(">>> disk_write: sector=%lu, count=%u (bytes=%u)\n", sector, count, count*512);

    // 1. 先批量擦除所有涉及的 4KB 扇区
    for (uint32_t addr = erase_start; addr < erase_end; addr += 4096) {
        W25Q128_WriteEnable();
        if (W25Q128_SectorErase(addr) != W25Q128_RESULT_OK) {
            // printf("!!! Erase failed at 0x%08X\n", addr);
            return RES_ERROR;
        }
        while (W25Q128_IsBusy());  // 等待擦除完成
    }

    // 2. 再一次性写入所有数据（跨页自动处理）
    if (W25Q128_BufferWrite((uint8_t*)buff, start_addr, count * 512) != W25Q128_RESULT_OK) {
        // printf("!!! BufferWrite failed\n");
        return RES_ERROR;
    }

    // printf(">>> disk_write SUCCESS\n");
    return RES_OK;
}
#endif
/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT res = RES_PARERR;

    if (pdrv != DEV_FLASH) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd)
    {
    case CTRL_SYNC:
        while (W25Q128_IsBusy());   // 必须等芯片完全空闲！
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
        *(LBA_t*)buff = W25Q128_CAPACITY / 512;  // 32768
        res = RES_OK;
        break;

    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512;
        res = RES_OK;
        break;

    case GET_BLOCK_SIZE:
        *(DWORD*)buff = 64;     // 关键！64KB 簇 → 128 个扇区
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
        break;
    }
    return res;
}