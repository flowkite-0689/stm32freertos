#include "../ff16/ff.h"
#include "../ff16/diskio.h"
#include <string.h>
#include "uart_dma.h"
#include "oled_print.h"
#include "oled.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "delay.h"
#ifndef FM_LFN
/* FM_LFN may not be defined in some FatFs versions; define as 0 to keep compatibility
   (no LFN flag) so code that ORs FM_LFN compiles. */
#define FM_LFN 0
#endif

#ifdef STM32F4XX
#include "stm32f4xx.h"
extern uint32_t get_systick(void);      // 你的 systick 获取函数
#endif

#define DEV_FLASH         0
#define TEST_FILE_1      "0:test1.txt"
#define TEST_FILE_2      "0:bench.dat"
#define TEST_DIR         "0:mydir"
#define TEST_LFN_FILE    "0:long_filename_test_2025.txt"  // Test long filename

#if FF_USE_LFN == 1
// FatFs requires this global buffer when FF_USE_LFN == 1
char lfn_buffer[FF_MAX_LFN + 1];  // +1 for '\0'
#endif

FATFS  fs;
FIL    file;
FRESULT fr;
DIR    dir;
FILINFO fno;

UINT   bw, br;
BYTE   work[FF_MAX_SS];                     // 格式化用的工作缓冲区

/* 32KB 缓冲区放在 CCMRAM（F4 只有 64KB，32KB 很安全） */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    __attribute__((section(".ccmram")))
#elif defined(__GNUC__)
    __attribute__((section(".ccmram")))
#endif
static uint8_t g_buffer[32 * 1024] __attribute__((aligned(4)));

void print_fresult(FRESULT fr)
{
    switch(fr) {
        case FR_OK:                      printf("FR_OK\n"); break;
        case FR_DISK_ERR:                printf("FR_DISK_ERR\n"); break;
        case FR_INT_ERR:                 printf("FR_INT_ERR\n"); break;
        case FR_NOT_READY:               printf("FR_NOT_READY\n"); break;
        case FR_NO_FILE:                 printf("FR_NO_FILE\n"); break;
        case FR_NO_PATH:                 printf("FR_NO_PATH\n"); break;
        case FR_INVALID_NAME:            printf("FR_INVALID_NAME\n"); break;
        case FR_DENIED:                  printf("FR_DENIED\n"); break;
        case FR_EXIST:                   printf("FR_EXIST\n"); break;
        case FR_INVALID_OBJECT:          printf("FR_INVALID_OBJECT\n"); break;
        case FR_WRITE_PROTECTED:         printf("FR_WRITE_PROTECTED\n"); break;
        case FR_INVALID_DRIVE:           printf("FR_INVALID_DRIVE\n"); break;
        case FR_NOT_ENABLED:             printf("FR_NOT_ENABLED\n"); break;
        case FR_NO_FILESYSTEM:           printf("FR_NO_FILESYSTEM\n"); break;
        case FR_MKFS_ABORTED:            printf("FR_MKFS_ABORTED\n"); break;
        case FR_TIMEOUT:                 printf("FR_TIMEOUT\n"); break;
        case FR_LOCKED:                  printf("FR_LOCKED\n"); break;
        case FR_NOT_ENOUGH_CORE:         printf("FR_NOT_ENOUGH_CORE\n"); break;
        case FR_TOO_MANY_OPEN_FILES:     printf("FR_TOO_MANY_OPEN_FILES\n"); break;
        case FR_INVALID_PARAMETER:       printf("FR_INVALID_PARAMETER\n"); break;
        default:                         printf("Unknown error: %d\n", fr); break;
    }
}

void filesystem_test(void)
{
    printf("\n========== W25Q128 FatFs Test Start ==========\n");
    
    /* 1. 挂载 */
    printf("Mounting filesystem...");
    OLED_Printf_Line(0,"W25Q128 FatFs Test Start ");
    OLED_Refresh();
    // Try mount first
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
         OLED_Printf_Line(1,"Formatting...");
         OLED_Refresh();
        printf("No valid filesystem or mount failed (%d). Formatting...\n", fr);

        MKFS_PARM opt = {0};
        opt.fmt      = FM_FAT32 | FM_SFD;  // SFD = Super Floppy (MBR not needed for flash)
        opt.au_size  = 32 * 1024;          // 32KB cluster (optimal for 16MB; max FAT32 cluster = 32KB)
        opt.align    = 0;
        opt.n_fat    = 2;                  // 2 FATs for safety (default)
        opt.n_root   = 0;                  // FAT32: root is a dir, not fixed entry count

        fr = f_mkfs("0:", &opt, work, sizeof(work));
        if (fr != FR_OK) {
            printf("f_mkfs failed: "); print_fresult(fr);
            return;
        }
        printf("Format OK! Remounting...\n");
        f_mount(NULL, "0:", 0);
        fr = f_mount(&fs, "0:", 1);
    }

    if (fr != FR_OK) {
        printf("Mount failed: "); print_fresult(fr);
        return;
    }
    printf("Mount success!\n");

    /* 2. 显示容量 */
    DWORD fre_clust, tot_sect, fre_sect;
    FATFS *fsp;
    if (f_getfree("0:", &fre_clust, &fsp) == FR_OK) {
        tot_sect = (fsp->n_fatent - 2) * fsp->csize;
        fre_sect = fre_clust * fsp->csize;
        printf("Total: %lu KB, Free: %lu KB\n", tot_sect/2, fre_sect/2);
    }

    /* 3. 创建目录 */
    printf("Creating directory...");
    fr = f_mkdir(TEST_DIR);
    printf((fr == FR_OK || fr == FR_EXIST) ? "OK\n" : "Failed: ");
    if (fr != FR_OK && fr != FR_EXIST) print_fresult(fr);

    /* 4. 写小文件 */
    printf("Writing small file %s...", TEST_FILE_1);
    fr = f_open(&file, TEST_FILE_1, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr == FR_OK) {
        const char *text = "Hello W25Q128 + FatFs! This is a test from 2025.\r\n";
        f_write(&file, text, strlen(text), &bw);
        f_close(&file);
        printf("OK (%d bytes)\n", bw);
    } else {
        printf("Open failed: "); print_fresult(fr);
    }

    /* 4b. 测试长文件名 */
    printf("Testing long filename %s...", TEST_LFN_FILE);
    OLED_Printf_Line(0,"Testing long filename %s...", TEST_LFN_FILE);
    OLED_Refresh();
    fr = f_open(&file, TEST_LFN_FILE, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr == FR_OK) {
        const char *lfn_text = "This is a test of long filename support.\r\n";
        f_write(&file, lfn_text, strlen(lfn_text), &bw);
        f_close(&file);
        printf("OK (%d bytes)\n", bw);
    } else {
        printf("LFN test failed: "); print_fresult(fr);
    }

    /* 5. 读回小文件 */
    printf("Reading back %s...\n", TEST_FILE_1);
    if (f_open(&file, TEST_FILE_1, FA_READ) == FR_OK) {
        char rbuf[128] = {0};
        f_read(&file, rbuf, sizeof(rbuf)-1, &br);
        f_close(&file);
        printf("Content:\n%s\n", rbuf);
    }

    /* 6. 1MB 性能测试 */
    printf("Writing 1MB benchmark file...");
    OLED_Printf_Line(0,"Writing 1MB benchmark file...");
    fr = f_open(&file, TEST_FILE_2, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) { printf("Open failed\n"); goto end; }

    memset(g_buffer, 0xA5, sizeof(g_buffer));
    uint32_t start = get_systick();
    int total_written = 0;

    for (int i = 0; i < 32; i++) {           // 32 × 32KB = 1MB
        fr = f_write(&file, g_buffer, sizeof(g_buffer), &bw);
        if (fr != FR_OK || bw != sizeof(g_buffer)) {
            printf("Write error at block %d\n", i);
            break;
        }
        total_written += bw;
    }
    f_close(&file);

    uint32_t elapsed = get_systick() - start;
    float seconds = elapsed / 1000.0f;
    if (seconds < 0.001f) seconds = 0.001f;
    float speed = total_written / 1024.0f / seconds;
    printf("1MB written in %.2f s -> %.1f KB/s\n", seconds, speed);
 OLED_Printf_Line(1,"1MB written in %.2f s -> %.1f KB/s\n", seconds, speed);
 OLED_Refresh();
    /* 7. 目录列表 */
    printf("\nDirectory listing:\n");
    if (f_opendir(&dir, "0:") == FR_OK) {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
            if (fno.fattrib & AM_DIR)
                printf(" <DIR>   %s\n", fno.fname);
            else
                printf("        %lu  %s\n", (unsigned long)fno.fsize, fno.fname);
        }
        f_closedir(&dir);
    }

end:
    f_mount(NULL, "0:", 0);
    printf("========== W25Q128 FatFs Test End ==========\n\n");
}