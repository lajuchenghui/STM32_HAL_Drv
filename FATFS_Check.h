#ifndef FATFS_CHECK_H
#define FATFS_CHECK_H

#include "ff.h"
#include "ff_gen_drv.h"
#include "user_diskio.h" /* defines USER_Driver as external */

#define FILE_TEST_BUF_NUM               17

uint32_t SPI_Flash_FATFS_Init(FATFS *fs, const char *disk_path);

uint32_t FATFS_mount(FATFS *fs, const TCHAR *path);
uint32_t FATFS_write_read_test(void);
uint32_t FATFS_seek_printf_test(FATFS *fs);
uint32_t FATFS_opendir_test(void);
uint32_t FATFS_scan_disk(char* path);
FRESULT FATFS_scan_files(char* path);

FRESULT fatfs_format_disk(FATFS *fs, const TCHAR *path);
void printf_fatfs_error(FRESULT fresult);

#endif
