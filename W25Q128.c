#include "W25Q128.h"

static uint32_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

volatile uint32_t W25_busy_ms;
volatile uint32_t W25_busy_flag;

/**
  * 函数功能: W25flash简单读写测试
  * 输入参数: 无
  * 返 回 值: 0 
  * 说    明: 无
  */
uint32_t W25_Read_write_Test(void)
{
  	uint8_t buf_w = 0x12;
	uint8_t buf_r[10] = {0};
	uint8_t buf_r2[10] = {0};
	
	//W25_Flash_Erase_Sector(0);
	W25_Flash_Write_Page(0, &buf_w, 1);
	W25_Flash_Read(0, buf_r, 2);
	
	W25_Flash_Read(256, buf_r, 2);
	W25_Flash_Erase_Sector(256);
	W25_Flash_Read(256, buf_r2, 2);
	W25_Flash_Read(258, buf_r2, 2);
	
	return 0;
}

/**
  * 函数功能: W25flash整片读写测试
  * 输入参数: 无
  * 返 回 值: 0 
  * 说    明: 无
  */
uint32_t W25_Read_write_Test_All(void)
{
	uint32_t i,k;
	uint8_t j = 0;
  	uint8_t buf_w[W25_SECTOR_SIZE] = {0};
	uint8_t buf_r[W25_SECTOR_SIZE] = {0};
	uint32_t res = 0;
	uint32_t err = 0;
	
	__wrn("W25_Read_write_Test_All begin\r\n");
	/*  buf_w 填充 0-255  */
	for(i=0;i<W25_SECTOR_SIZE;i++)
	{
		buf_w[i] = j;
		j++;
	}
	
	for(k=0;k<W25_SECTOR_COUNT;k++)
	{
		__wrn("sector count:0x%x current test sector:0x%x\r\n", W25_SECTOR_COUNT, k+1);
		
		W25_Flash_Write_Sector(k, buf_w);
		W25_Flash_Read_Sector(k, buf_r);
	
		res = Buffercmp(buf_w, buf_r, W25_SECTOR_SIZE);
		if(res == 0)
		{
		  __wrn("sector:0x%x cmp err  err cnt:0x%x\r\n", k + 1, err);
			err++;
		}
		else
		{
			__wrn("sector:0x%x cmp secc  err cnt:0x%x\r\n", k + 1, err);
		}
	}
	__wrn("sector cmp done count:0x%x err cnt:0x%x\r\n", W25_SECTOR_COUNT, err);
	
	return 0;
}

/*  FATFS IO head  */
/**
  * 函数功能: W25 FATFS 初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: FATFS对disk设备的初始化会被多次调用 所用要有个标记防止多次被调用
  */
void W25_FATFS_Init(void)
{
  	static uint32_t init_flag = 0;
	
	if(init_flag == 0)
	{
		init_flag = 1;
		
		/*  偏移  */
		//W25_Flash_Erase_Sector(SPI_FLASH_FAT_START_SECTOR * W25_SECTOR_SIZE);
	}	
}

/**
  * 函数功能: W25 FATFS 状态读取
  * 输入参数: 无
  * 返 回 值: 状态异常：0 状态正常：1 
  * 说    明: FATFS会频繁的调用此函数 保证disk设备是随时可操作的
  */
uint32_t W25_FATFS_Sta(void)
{
	return 1;
}

/**
  * 函数功能: W25 FATFS 读取接口
  * 输入参数: buf：读取数据 sector：读取扇区地址 cnt：读取长度
  * 返 回 值: 1
  * 说    明: 无
  */
uint32_t W25_FATFS_Read(uint8_t *buff, uint32_t sector, uint32_t cnt)
{
	uint32_t i = 0;
	
	/*  扇区偏移  */
	sector += SPI_FLASH_FAT_START_SECTOR;
	for(i=0;i<cnt;i++)
	{
		W25_Flash_Read_Sector(sector, buff);
		sector++;
		buff += W25_SECTOR_SIZE;
	}
	
	return 1;
}

/**
  * 函数功能: W25 FATFS 写入接口
  * 输入参数: buf：写入数据 sector：写入扇区地址 cnt：写入长度
  * 返 回 值: 1
  * 说    明: 无
  */
uint32_t W25_FATFS_Write(const uint8_t *buff, uint32_t sector, uint32_t cnt)
{
	uint32_t i = 0;
	
	/*  扇区偏移  */
	sector += SPI_FLASH_FAT_START_SECTOR;
	for(i=0;i<cnt;i++)
	{
		W25_Flash_Write_Sector(sector, buff);
		sector++;
		buff += W25_SECTOR_SIZE;
	}
	
	return 1;
}
/*  FATFS IO tail  */

/*  USB storage IO head  */

/**
  * 函数功能: W25 U disk 读取接口
  * 输入参数: buf：读取数据 blk_addr：读取地址 blk_len：读取长度
  * 返 回 值: 1
  * 说    明: 无
  */
uint32_t W25_U_Storage_Read(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	uint32_t i;
  
    /*  扇区偏移  */
    blk_addr += SPI_FLASH_FAT_START_SECTOR;
    for(i=0;i<blk_len;i++)
    {
        W25_Flash_Read_Sector(blk_addr, buf);
        blk_addr++;
        buf += W25_SECTOR_SIZE;
    }
    __wrn("STORAGE_Read_FS secc blk_addr == 0x%x blk_len == 0x%x\r\n", blk_addr + SPI_FLASH_FAT_START_SECTOR, blk_len);
    
    return 1;
}

/**
  * 函数功能: W25 U disk 写入接口
  * 输入参数: buf：写入数据 blk_addr：写入地址 blk_len：写入长度
  * 返 回 值: 1
  * 说    明: 无
  */
uint32_t W25_U_Storage_Write(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    uint32_t i;
  
    /*  扇区偏移  */
    blk_addr += SPI_FLASH_FAT_START_SECTOR;
    for(i=0;i<blk_len;i++)
    {
        W25_Flash_Write_Sector(blk_addr, buf);
        blk_addr++;
        buf += W25_SECTOR_SIZE;
    }
    __wrn("STORAGE_Write_FS secc blk_addr == 0x%x blk_len == 0x%x\r\n", blk_addr + SPI_FLASH_FAT_START_SECTOR, blk_len);
    
    return 1;
}
/*  USB storage IO tail  */

/**
  * 函数功能: W25毫秒计数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_sys_tick_ms(void)
{
	if(W25_busy_flag == 1)
	{
		W25_busy_ms++;
	}
}

/**
  * 函数功能: W25读设备ID
  * 输入参数: 无
  * 返 回 值: 设备ID
  * 说    明: 无
  */
uint32_t W25_Read_Device_ID(void)
{
	uint32_t c = W25_DEVICE_ID;
	uint32_t c0 = 0;
	
  	uint32_t ID = 0;

	SPI_FLASH_DISABLE();
	SPI_FLASH_ENABLE();
	W25_SPI_RW_Byte(W25_DEVICE_ID);
	W25_SPI_RW_Byte(0);
	W25_SPI_RW_Byte(0);
	W25_SPI_RW_Byte(0);
	/*  MANUFACTURER ID  :Winbond Serial Flash EFh  */
	ID = W25_SPI_RW_Byte(0);
	/*  Device ID 1Byte  */
	ID = (ID << 8) | W25_SPI_RW_Byte(0);
	
	SPI_FLASH_DISABLE();
	__msg("W25_Read_Device_ID == 0x%x\r\n", ID);
	return ID;
}

/**
  * 函数功能: W25读唯一ID
  * 输入参数: 无
  * 返 回 值: 唯一ID
  * 说    明: 无
  */
uint32_t W25_Read_Unique_ID(void)
{
	uint32_t i;
	uint32_t ID = 0;
	
	SPI_FLASH_DISABLE();
	SPI_FLASH_ENABLE();
	
	W25_SPI_RW_Byte(W25_UNIQUE_ID);
	for(i = 0;i < 4;i++)
	{
		W25_SPI_RW_Byte(0);
	}
	ID = W25_SPI_RW_Byte(0);
	ID = (ID << 8) | W25_SPI_RW_Byte(0);
	ID = (ID << 8) | W25_SPI_RW_Byte(0);
	ID = (ID << 8) | W25_SPI_RW_Byte(0);
	
	SPI_FLASH_DISABLE();
	__msg("W25_Read_Unique_ID == 0x%x\r\n", ID);
	return ID;
}

/**
  * 函数功能: W25读兼容性ID
  * 输入参数: 无
  * 返 回 值: 兼容性ID
  * 说    明: 无
  */
uint32_t W25_Read_JEDEC_ID(void)
{
  	uint32_t ID = 0;
	
	SPI_FLASH_ENABLE();
	
	W25_SPI_RW_Byte(W25_JEDEC_ID);
	/*  MANUFACTURER ID  :Winbond Serial Flash EFh  */
	ID = (ID << 8) | W25_SPI_RW_Byte(0);
	/*  Memory Type ID15-8  */
	ID = (ID << 8) | W25_SPI_RW_Byte(0);
	/*  Capacity ID7-0  */
	ID = (ID << 8) | W25_SPI_RW_Byte(0);
	
	SPI_FLASH_DISABLE();
	__msg("W25_Read_JEDEC_ID == 0x%x\r\n", ID);
	return ID;
}

/**
  * 函数功能: W25flash扇区读取
  * 输入参数: addr：扇区地址 即第几个扇区 0-4095 data：读取数据
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_Flash_Read_Sector(uint32_t addr, uint8_t *data)
{
	if(addr > 4095)
	{
		while(1);
	}
	__msg("Flash read sector:0x%x\r\n", addr);
	W25_Flash_Read(addr * W25_SECTOR_SIZE, data, W25_SECTOR_SIZE);

}

/**
  * 函数功能: W25flash扇区擦除写入
  * 输入参数: addr：扇区地址 即第几个扇区 0-4095 data：写入数据
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_Flash_Write_Sector(uint32_t addr, const uint8_t *data)
{
  	uint32_t tmp;
	if(addr > 4095)
	{
		while(1);
	}
	tmp = addr * W25_SECTOR_SIZE;
	__msg("Flash write sector:0x%x\r\n", addr);
	W25_Flash_Erase_Sector(tmp);
	W25_Flash_Write(tmp, (uint8_t *)data, W25_SECTOR_SIZE);
}

/**
  * 函数功能: W25flash读取
  * 输入参数: addr：读取地址 data：读取数据 len：读取长度
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_Flash_Read(uint32_t addr, uint8_t *data, uint32_t len)
{
  	uint32_t i;
	
	W25_Flash_Write_Enable();
	SPI_FLASH_DISABLE();
	SPI_FLASH_ENABLE();
	W25_SPI_RW_Byte(W25_READ_DATA);
	W25_SPI_RW_Byte((uint8_t)(addr >> 16));
	W25_SPI_RW_Byte((uint8_t)(addr >> 8));
	W25_SPI_RW_Byte((uint8_t)addr);
	//__wrn("Flash read addr = 0x%x data:", addr);
	for(i=0;i<len;i++)
	{
		data[i] = W25_SPI_RW_Byte(0xff);
		//printf("0x%x ", data[i]);
		//HAL_Delay(1);
	}
	//printf("\r\n");
	SPI_FLASH_DISABLE();
}

/**
  * 函数功能: W25flash写入
  * 输入参数: addr：写入地址 data：写入数据 len：写入长度
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_Flash_Write(uint32_t addr, uint8_t *data, uint32_t len)
{
  	uint32_t i;
	uint32_t page_num = len >> 8;
	uint32_t relase_num = len & 0xff;
	
	if(len > 256)
	{
		for(i=0;i<page_num;i++)
		{
			W25_Flash_Write_Page(addr + (i * 256), (uint8_t *)(data + (i * 256)), 256);
		}
		if(relase_num != 0)
		{
			W25_Flash_Write_Page(addr + (i * 256), (uint8_t *)(data + (i * 256)), relase_num);
		}
	}
	else
	{
		W25_Flash_Write_Page(addr, data, len);
	}
	
}

/**
  * 函数功能: W25flash页写
  * 输入参数: addr：页所在地址 data：写入数据 len：写入长度 最大256
  * 返 回 值: 写入失败：0 写入成功：1
  * 说    明: 无
  */
uint32_t W25_Flash_Write_Page(uint32_t addr, uint8_t *data, uint32_t len)
{
	uint32_t i;
	
	W25_Flash_Write_Enable();
	SPI_FLASH_ENABLE();
	W25_SPI_RW_Byte(W25_PAGE_PROGRAM);
	W25_SPI_RW_Byte((uint8_t)(addr >> 16));
	W25_SPI_RW_Byte((uint8_t)(addr >> 8));
	W25_SPI_RW_Byte((uint8_t)addr);
	
	//__wrn("Flash write addr = 0x%x data:", addr);
	for(i=0;i<len;i++)
	{
		W25_SPI_RW_Byte(data[i]);
		//printf("0x%x ", data[i]);
		//HAL_Delay(1);
	}
	//printf("\r\n");
	SPI_FLASH_DISABLE();
	
	return W25_Wait_Busy(1);
}

/**
  * 函数功能: W25flash擦除整片
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_Flash_Erase_Chip(void)
{
	W25_Flash_Write_Enable();
	W25_Wait_Busy(1);
	SPI_FLASH_ENABLE();
	W25_SPI_RW_Byte(W25_CHIP_ERASE);
	SPI_FLASH_DISABLE();
	W25_Wait_Busy(100);
}

/**
  * 函数功能: W25flash擦除一个扇区
  * 输入参数: addr：要擦除扇区的地址
  * 返 回 值: 擦除失败：0 擦除成功：1
  * 说    明: 无
  */
uint32_t W25_Flash_Erase_Sector(uint32_t addr)
{
  	uint8_t buf = 0;
	
  	W25_Flash_Write_Enable();
	SPI_FLASH_DISABLE();
	SPI_FLASH_ENABLE();
	
	W25_SPI_RW_Byte(W25_BLOCK_SECTOR_UNLOCK);
	W25_SPI_RW_Byte((uint8_t)(addr >> 16));
	W25_SPI_RW_Byte((uint8_t)(addr >> 8));
	W25_SPI_RW_Byte((uint8_t)addr);
	
	SPI_FLASH_DISABLE();
	W25_Flash_Write_Enable();
	W25_Wait_Busy(1);
	SPI_FLASH_ENABLE();
	
	W25_SPI_RW_Byte(W25_SECTOR_ERASE);
	W25_SPI_RW_Byte((uint8_t)(addr >> 16));
	W25_SPI_RW_Byte((uint8_t)(addr >> 8));
	W25_SPI_RW_Byte((uint8_t)addr);
	
	SPI_FLASH_DISABLE();
	W25_Wait_Busy(2);
	
	W25_Flash_Read(addr, &buf, 1);
	if(buf == (uint32_t)0xff)
	{
		return 1;
	}
	return 0;
}

/**
  * 函数功能: W25flash操作等待完成
  * 输入参数: ms：最大等待时间 * 300
  * 返 回 值: 等待响应失败：0 等待响应成功：1
  * 说    明: 无
  */
uint32_t W25_Wait_Busy(uint32_t ms)
{
	uint32_t i = 0;
	uint32_t sta;
	
	while(((W25_Read_SR123(1) & 0x01) == 0x01) && (i++ < 300))
	{
		W25_busy_flag = 1;
		W25_busy_ms = 0;
		while(W25_busy_ms < ms);
		W25_busy_flag = 0;
		
	}
	
	if(i > 300)
	{
		return 0;
	}
	
	return 1;
}

/**
  * 函数功能: W25flash写失能
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_Flash_Write_Enable(void)
{
  	SPI_FLASH_ENABLE();
	W25_SPI_RW_Byte(W25_WRITE_ENABLE);
	SPI_FLASH_DISABLE();
}

/**
  * 函数功能: W25flash写失能
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void W25_Flash_Write_Disable(void)
{
	SPI_FLASH_ENABLE();
	W25_SPI_RW_Byte(W25_WRITE_DISABLE);
	SPI_FLASH_DISABLE();
}

/**
  * 函数功能: 读W25的状态寄存器
  * 输入参数: val：要读的状态寄存器
  * 返 回 值: ret：返回的值
  * 说    明: 无
  */
uint32_t W25_Read_SR123(uint32_t val)
{
	uint32_t ret;
	
	SPI_FLASH_DISABLE();
	SPI_FLASH_ENABLE();
	
	if(val == 1)
	{
		W25_SPI_RW_Byte(W25_READ_STA_REG_1);
	}
	else if(val == 2)
	{
		W25_SPI_RW_Byte(W25_READ_STA_REG_2);
	}
	else if(val == 2)
	{
		W25_SPI_RW_Byte(W25_READ_STA_REG_3);
	}
	else
	{
		return 0;
	}
	
	ret = W25_SPI_RW_Byte(0);

	SPI_FLASH_DISABLE();
	
	return ret;
}

/**
  * 函数功能: SPI读写单个字节
  * 输入参数: send：写一个字节
  * 返 回 值: rec：返回一个字节
  * 说    明: 无
  */
uint32_t W25_SPI_RW_Byte(uint32_t send)
{
  	uint32_t rec = 0;
	
	if(HAL_SPI_TransmitReceive(&SPI_HANDLE, (uint8_t *)&send, (uint8_t *)&rec, 1, 0x200) != HAL_OK)
	{
		while(1);
	}
	
	return rec;
}

/**
  * 函数功能: 比较两个缓冲区中的数据是否相等
  * 输入参数: pBuffer1：要比较的缓冲区1的指针
  *           pBuffer2：要比较的缓冲区2的指针
  *           BufferLength：缓冲区长度
  * 返 回 值: 1：相等
  *           0：不等
  * 说    明: 无
  */
static uint32_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return 0;
    }

    pBuffer1++;
    pBuffer2++;
  }
  return 1;
}
