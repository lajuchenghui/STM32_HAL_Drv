#ifndef W25Q125_H
#define W25Q125_H

/*
The W25Q128FV array is organized into 65,536 programmable pages of 256-bytes each. Up to 256 bytes
can be programmed at a time. 
Pages can be erased in groups of 16 (4KB sector erase), groups of 128(32KB block erase),
groups of 256 (64KB block erase) or the entire chip (chip erase). 
The W25Q128FV has 4,096 erasable sectors and 256 erasable blocks respectively. 
The small 4KB sectors allow for greater flexibility in applications that require data and parameter storage.


MANUFACTURER ID  :Winbond Serial Flash EFh

Device ID				(ID7 - ID0)				(ID15 - ID0)
Instruction				ABh, 90h, 92h, 94h		9Fh
W25Q128FV (SPI Mode)	17h						4018h
W25Q128FV (QPI Mode)  	17h  					6018h

000000h - 00FFFFh 64KBytes block 0
...
FF0000h - FFFFFFh 64KBytes block 255
*/

#include "stm32f4xx_hal.h"
#include "dbg_printf.h"

#define SPI_HANDLE 								hspi5


#define W25_PAGE_SIZE       			256
#define W25_SECTOR_SIZE     			4096
 
#define W25_SECTOR_COUNT				4096
#define W25_BLOCK_SIZE					65536
/*  这两个加起来必须等于4096  */
#define SPI_FLASH_FAT_START_SECTOR   	0       // 在使用FATFS和Udisk IO操作flash的 偏移量
#define SPI_FLASH_FAT_SECTOR_COUNT   	4096    // 除去偏移量的扇区个数 

#define SPI_FLASH_ENABLE()						HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_RESET)
#define SPI_FLASH_DISABLE()						HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET)


#define W25_PAGE_PROGRAM				0x02
#define W25_WRITE_DISABLE				0x04
#define W25_WRITE_ENABLE				0x06

#define W25_READ_STA_REG_1				0x05
#define W25_READ_STA_REG_2				0x35
#define W25_READ_STA_REG_3				0x15
/*  Read Manufacturer / Device ID (90h)  */
/*  Read Manufacturer / Device ID Dual I/O (92h)  */
/*  Read Manufacturer / Device ID Quad I/O (94h)  */
#define W25_DEVICE_ID 					0x90
/*  Read Unique ID Number (4Bh)  */
#define W25_UNIQUE_ID					0x4b
/*  Read JEDEC ID (9Fh)  */
#define W25_JEDEC_ID 					0x9f

#define W25_BLOCK_SECTOR_LOCK			0x36
#define W25_BLOCK_SECTOR_UNLOCK			0x39
/*  4KBytes  */
#define W25_SECTOR_ERASE				0x20
/*  32KBytes  */
#define W25_BLOCK_ERASE32				0x52
/*  64KBytes  */
#define W25_BLOCK_ERASE64				0xD8
#define W25_CHIP_ERASE					0xc7

#define W25_READ_DATA					0x03

#define W25_SECTOR_NUM1_ADDR			0
#define W25_SECTOR_NUM2_ADDR			0x1000
#define W25_SECTOR_NUM3_ADDR			0x2000
#define W25_SECTOR_NUM4_ADDR			0x3000
#define W25_SECTOR_NUM5_ADDR			0x4000


extern SPI_HandleTypeDef SPI_HANDLE;

uint32_t W25_Read_write_Test(void);
uint32_t W25_Read_write_Test_All(void);
uint32_t W25_Read_Device_ID(void);
uint32_t W25_Read_JEDEC_ID(void);
uint32_t W25_Read_Unique_ID(void);
/*  FATFS IO head  */
void W25_FATFS_Init(void);
uint32_t W25_FATFS_Sta(void);
uint32_t W25_FATFS_Read(uint8_t *buff, uint32_t sector, uint32_t cnt);
uint32_t W25_FATFS_Write(const uint8_t *buff, uint32_t sector, uint32_t cnt);
/*  FATFS IO tail  */

/*  USB storage IO head  */
uint32_t W25_U_Storage_Read(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
uint32_t W25_U_Storage_Write(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);

/*  USB storage IO tail  */

void W25_sys_tick_ms(void);
void W25_Flash_Read_Sector(uint32_t addr, uint8_t *data);
void W25_Flash_Write_Sector(uint32_t addr, const uint8_t *data);
void W25_Flash_Write(uint32_t addr, uint8_t *data, uint32_t len);
void W25_Flash_Read(uint32_t addr, uint8_t *data, uint32_t len);
uint32_t W25_Flash_Write_Page(uint32_t addr, uint8_t *data, uint32_t len);
void W25_Flash_Erase_Chip(void);
uint32_t W25_Flash_Erase_Sector(uint32_t addr);
uint32_t W25_Wait_Busy(uint32_t ms);
void W25_Flash_Write_Enable(void);
void W25_Flash_Write_Disable(void);
uint32_t W25_Read_SR123(uint32_t val);
uint32_t W25_SPI_RW_Byte(uint32_t send);

#endif
