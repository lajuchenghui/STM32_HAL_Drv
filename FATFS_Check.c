#include "FATFS_Check.h"

/**
  * 函数功能: 文件系统初始化
  * 输入参数: FATFS文件系统 path文件系统路径
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t SPI_Flash_FATFS_Init(FATFS *fs, const char *disk_path)
{
    /*  path空间大小影响文件扫描 当存在文件名大于path空间大小 程序卡死  */
  
    if(FATFS_mount(fs, disk_path) != 1)return 0;
        
    if(FATFS_write_read_test(disk_path) != 1)return 0;
	
    if(FATFS_seek_printf_test(fs, disk_path) != 1)return 0;

    if(FATFS_opendir_test(disk_path) != 1)return 0;
    
    FATFS_scan_disk(disk_path);
    
    return 1;
    /*  fat write read test end  */
}

uint32_t SDRAM_FATFS_Init(FATFS *fs, const char *disk_path)
{
	uint32_t tot,fre;
	
	
	FATFS_mount(fs, disk_path);
    FATFS_format_disk(fs, disk_path);
	FATFS_file_cpy(disk_path, (const char *)USERPath, "img_p1.bin");
	FATFS_file_cpy(disk_path, (const char *)USERPath, "img_p2.bin");
	FATFS_scan_disk(disk_path);
		
	FATFS_getfree(fs, disk_path , &tot, &fre);

	return 1;
}

uint32_t FATFS_file_to_flash(uint32_t dst_addr, const char *src_path, const char *file_name)
{
	FIL file_src;
	FRESULT f_res; 
	uint32_t fnum;
	uint32_t file_size;
	uint32_t cpy_cnt;
	uint32_t cpy_remainder;
	uint32_t i,j;
	/*  放在SDRAM 最大256KBytes  */
	uint32_t buf_cpy[FILE_CPY_BUF_SIZE];
	
	uint8_t src_file_path[40] = {0};
    uint8_t dst_file_path[40] = {0};
	
	__msg("FATFS_file_to_flash dst_addr == 0x%x src_path == %s file_name == %s\r\n", dst_addr, src_path, file_name);
	if((src_path == NULL) || (file_name == NULL))
	{
		__msg("FATFS_file_cpy arg err\r\n");
		return 0;
	}
	
	sprintf(src_file_path, "%s%s", src_path, file_name);
	/*  打开源文件  */
	f_res = f_open(&file_src, src_file_path, FA_OPEN_EXISTING | FA_READ);
	if(f_res != FR_OK)
    {
        __msg("FATFS open source file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	
	/*  得到大小并分割  */
    file_size = f_size(&file_src);
    __msg("file size == %lu B\r\n", file_size);
	cpy_cnt = file_size / FILE_CPY_BUF_SIZE;
	cpy_remainder = file_size % FILE_CPY_BUF_SIZE;
	__msg("FATFS_file_cpy source file cpy_cnt == %d cpy_remainder == %d\r\n", cpy_cnt, cpy_remainder);
	/*  要复制的大小大于缓冲 需要分多次来复制  */
	if(file_size > FILE_CPY_BUF_SIZE)
	{
		for(i=0;i<cpy_cnt;i++)
		{
            __msg("f_read start\r\n");
			f_res = f_read(&file_src, buf_cpy, FILE_CPY_BUF_SIZE, &fnum); 
			if(f_res != FR_OK)
			{
				__msg("FATFS read file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS read file secc\r\n");
			__msg("source file read bytes == %u B\r\n", fnum);
			
			//write	flash
            W25_Flash_Write_Sector(dst_addr + i, buf_cpy);
			
			f_res = f_lseek(&file_src, FILE_CPY_BUF_SIZE * (i + 1));
			if(f_res != FR_OK)
			{
				__msg("FATFS seek file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS seek file secc\r\n");
			
		}
		
		if(cpy_remainder != 0)
		{   
            /*  不够一个扇区的 填充1个扇区  */
            for(j=0;j<FILE_CPY_BUF_SIZE-cpy_remainder;j++)
            {
                buf_cpy[cpy_remainder + j] = 0xff;
            }
            
			f_res = f_read(&file_src, buf_cpy, cpy_remainder, &fnum); 
			if(f_res != FR_OK)
			{
				__msg("FATFS read file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS read file secc\r\n");
			__msg("source file read bytes == %u B\r\n", fnum);
			
            //write	flash
            W25_Flash_Write_Sector(dst_addr + i, buf_cpy);
            
			__msg("destination file write bytes == %u B\r\n", fnum);	
		}
		
	}
    /*  要复制的大小 小于或等于缓冲 不需要分多次来复制  */
	else if(file_size != 0)
	{
        /*  不够一个扇区的 填充1个扇区  */
        for(j=0;j<FILE_CPY_BUF_SIZE-file_size;j++)
        {
            buf_cpy[file_size + j] = 0xff;
        }
            
		f_res = f_read(&file_src, buf_cpy, file_size, &fnum); 
		if(f_res != FR_OK)
		{
			__msg("FATFS read file fail\r\n");
			printf_fatfs_error(f_res);
			return 0;
		}
		__msg("FATFS read file secc\r\n");
		__msg("source file read bytes == %u B\r\n", fnum);
		
		//write	flash
        W25_Flash_Write_Sector(dst_addr + i + 1, buf_cpy);
		__msg("destination file write bytes == %u B\r\n", fnum);
		
	}
	/*  关闭文件  */
	f_res = f_close(&file_src);
	if(f_res != FR_OK)
    {
        __msg("FATFS clost file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS clost file secc\r\n");


	return 1;
}

uint32_t FATFS_file_cpy(const char *dst_path, const char *src_path, const char *file_name)
{
	FIL file_dst;
	FIL file_src;
	FRESULT f_res; 
	uint32_t fnum;
	uint32_t file_size;
	uint32_t cpy_cnt;
	uint32_t cpy_remainder;
	uint32_t i;
	/*  放在SDRAM 最大256KBytes  */
	uint32_t buf_cpy[FILE_CPY_BUF_SIZE];
	
	uint8_t src_file_path[40] = {0};
    uint8_t dst_file_path[40] = {0};
	
	__msg("FATFS_file_cpy dst_path == %s src_path == %s file_name == %s\r\n", dst_path, src_path, file_name);
	if((dst_path == NULL) || (src_path == NULL) || (file_name == NULL))
	{
		__msg("FATFS_file_cpy arg err\r\n");
		return 0;
	}
	
	sprintf(dst_file_path, "%s%s", dst_path, file_name);
	sprintf(src_file_path, "%s%s", src_path, file_name);
	/*  打开源文件  */
	f_res = f_open(&file_src, src_file_path, FA_OPEN_EXISTING | FA_READ);
	if(f_res != FR_OK)
    {
        __msg("FATFS open source file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS open source file secc\r\n");
	/*  打开目标文件  */
	f_res = f_open(&file_dst, dst_file_path, FA_CREATE_ALWAYS  | FA_WRITE);
	if(f_res != FR_OK)
    {
        __msg("FATFS open destination file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS open destination file secc\r\n");
	/*  得到大小并分割  */
    file_size = f_size(&file_src);
    __msg("file size == %u B\r\n", file_size);
	cpy_cnt = file_size / FILE_CPY_BUF_SIZE;
	cpy_remainder = file_size % FILE_CPY_BUF_SIZE;
	__msg("FATFS_file_cpy source file cpy_cnt == %d cpy_remainder == %d\r\n", cpy_cnt, cpy_remainder);
	/*  要复制的大小大于缓冲 需要分多次来复制  */
	if(file_size > FILE_CPY_BUF_SIZE)
	{
		for(i=0;i<cpy_cnt;i++)
		{
			f_res = f_read(&file_src, buf_cpy, FILE_CPY_BUF_SIZE, &fnum); 
			if(f_res != FR_OK)
			{
				__msg("FATFS read file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS read file secc\r\n");
			__msg("source file read bytes == %u B\r\n", fnum);
			
			f_res = f_write(&file_dst, buf_cpy, FILE_CPY_BUF_SIZE, &fnum); 
			if(f_res != FR_OK)
			{
				__msg("FATFS write file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS write file secc\r\n");
			__msg("destination file write bytes == %u B\r\n", fnum);	
			
			f_res = f_lseek(&file_src, FILE_CPY_BUF_SIZE * (i + 1));
			if(f_res != FR_OK)
			{
				__msg("FATFS seek file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS seek file secc\r\n");
			
			f_res = f_lseek(&file_dst, FILE_CPY_BUF_SIZE * (i + 1));
			if(f_res != FR_OK)
			{
				__msg("FATFS seek file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS seek file secc\r\n");
		}
		
		if(cpy_remainder != 0)
		{
			f_res = f_read(&file_src, buf_cpy, cpy_remainder, &fnum); 
			if(f_res != FR_OK)
			{
				__msg("FATFS read file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS read file secc\r\n");
			__msg("source file read bytes == %u B\r\n", fnum);
			
			f_res = f_write(&file_dst, buf_cpy, cpy_remainder, &fnum); 
			if(f_res != FR_OK)
			{
				__msg("FATFS write file fail\r\n");
				printf_fatfs_error(f_res);
				return 0;
			}
			__msg("FATFS write secc fail\r\n");
			__msg("destination file write bytes == %u B\r\n", fnum);	
		}
		
	}
    /*  要复制的大小小于或等于缓冲 不需要分多次来复制  */
	else
	{
		f_res = f_read(&file_src, buf_cpy, file_size, &fnum); 
		if(f_res != FR_OK)
		{
			__msg("FATFS read file fail\r\n");
			printf_fatfs_error(f_res);
			return 0;
		}
		__msg("FATFS read file secc\r\n");
		__msg("source file read bytes == %u B\r\n", fnum);
		
		f_res = f_write(&file_dst, buf_cpy, file_size, &fnum); 
		if(f_res != FR_OK)
		{
			__msg("FATFS write file fail\r\n");
			printf_fatfs_error(f_res);
			return 0;
		}
		__msg("FATFS write secc fail\r\n");
		__msg("destination file write bytes == %u B\r\n", fnum);
		
	}
	/*  关闭文件  */
	f_res = f_close(&file_src);
	if(f_res != FR_OK)
    {
        __msg("FATFS clost file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS clost file secc\r\n");
	f_res = f_close(&file_dst);
	if(f_res != FR_OK)
    {
        __msg("FATFS clost file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS clost file secc\r\n");

	return 1;
}

/*
FATFS系统会占用0-0x46的扇区（0-70）
*/
uint32_t FATFS_sector_cpy(const Diskio_drvTypeDef *dst, const Diskio_drvTypeDef *src, uint32_t num)
{
    uint32_t i;
    uint8_t buf[4096];
    
    for(i=0;i<num;i++)
    {   
        src->disk_read(0, buf, i, 1);
        dst->disk_write(0, buf, i, 1);
    }
    
	return 1;
}
/**
  * 函数功能: 文件系统挂载
  * 输入参数: FATFS文件系统 path文件系统路径
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_mount(FATFS *fs, const char *path)
{
    FIL file;
    FRESULT f_res; 
    const char test_file[] = "test.txt";
    char test_file_path[20] = {0};
    
    /*  fat mount  */
    __msg("////////////////////////////////////\r\n"); 
    __msg("FATFS mount start\r\n"); 
	f_res = f_mount(fs, path, 0);
	
    /*  没有文件系统 开始格式化  */
    if(f_res == FR_NO_FILESYSTEM)
    {
        f_res = FATFS_format_disk(fs, path);
        
        if(f_res != FR_OK)
        {
            printf_fatfs_error(f_res); 
            return 0;
        }
    }
    else if(f_res == FR_OK)
    {
        __msg("FATFS mount secc\r\n"); 
    }
    else
    {
        printf_fatfs_error(f_res);
        return 0;
    }
    
    sprintf(test_file_path, "%s%s", path, test_file);
    f_res = f_open(&file, test_file_path,FA_CREATE_ALWAYS | FA_WRITE );
    if(f_res == FR_NO_FILESYSTEM)
    {
        printf_fatfs_error(f_res);   
        __msg("FATFS open fail start format\r\n"); 
        f_res = FATFS_format_disk(fs, path);
        if(f_res != FR_OK)
        {
            printf_fatfs_error(f_res); 
            return 0;
        }
    }else if(f_res == FR_OK)
    {
        __msg("FATFS open file secc\r\n"); 
        
        f_res = f_close(&file);
        if(f_res != FR_OK)
        {
            __msg("FATFS close file fail\r\n");
            printf_fatfs_error(f_res);
            
            return 0;
        }
    }
    else
    {
        printf_fatfs_error(f_res);
    } 
    
    if(f_res != FR_OK)
    {
        __msg("FATFS open file fail\r\n");
        printf_fatfs_error(f_res);
        
        return 0;
    }
    
    __msg("FATFS mount end\r\n"); 
    __msg("////////////////////////////////////\r\n"); 
    return 1;
}

/**
  * 函数功能: 文件读写测试
  * 输入参数: 无
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_write_read_test(const char *path)
{
    FIL file;
	FRESULT f_res; 
	UINT fnum;
    const char test_file[] = "test.txt";
    char test_file_path[20] = {0};
	uint8_t file_write_buff[FILE_TEST_BUF_NUM] = "0123456789abcdef";
	uint8_t file_read_buff[FILE_TEST_BUF_NUM] = {0};
    
    sprintf(test_file_path, "%s%s", path, test_file);
    __msg("////////////////////////////////////\r\n"); 
    __msg("FATFS write read test start\r\n");
	f_res = f_open(&file, test_file_path,FA_CREATE_ALWAYS | FA_WRITE );
    
    if(f_res != FR_OK)
    {
        __msg("FATFS open file fail\r\n");
        printf_fatfs_error(f_res);
        
        return 0;
    }
    
    __msg("FATFS open create file secc\r\n");
    
    f_res = f_write(&file, file_write_buff, FILE_TEST_BUF_NUM, &fnum);
    
    if(f_res != FR_OK)
    {
        __msg("FATFS write file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    __msg("FATFS write file secc\r\n");
    __msg("write num == %d\r\n", FILE_TEST_BUF_NUM);
    __msg("write secc num == %d\r\n", fnum);
    
	f_res = f_close(&file);
    
    if(f_res != FR_OK)
    {
        __msg("FATFS close file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    __msg("FATFS close file secc\r\n");
    
    /*  fat read test  */
	__msg("FATFS read test\r\n");
	f_res = f_open(&file, test_file_path, FA_OPEN_EXISTING | FA_READ);
    
    if(f_res != FR_OK)
    {
        __msg("FATFS open file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
   
    __msg("FATFS open file secc\r\n");
    f_res = f_read(&file, file_read_buff, FILE_TEST_BUF_NUM, &fnum); 
    
    if(f_res != FR_OK)
    {
        __msg("FATFS read file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    __msg("FATFS read file secc\r\n");
    __msg("read num == %d\r\n", FILE_TEST_BUF_NUM);
	__msg("read secc num == %d\r\n", fnum);
    
	f_res = f_close(&file);
    
	if(f_res != FR_OK)
    {
        __msg("FATFS close file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    __msg("FATFS close file secc\r\n");
    __msg("FATFS write read test end\r\n");
    __msg("////////////////////////////////////\r\n"); 
    return 1;
}

/**
  * 函数功能: 文件定位和格式化写入功能测试
  * 输入参数: FatFS文件系统
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_seek_printf_test(FATFS *fs, const char *path)
{
    FIL file;
	FRESULT f_res; 
	UINT fnum;
    DWORD tot_sect;
    DWORD fre_sect;
    uint8_t readbuffer[512] = {0};
	const char test_file[] = "test2.txt";
    char test_file_path[20] = {0};
    
	sprintf(test_file_path, "%s%s", path, test_file);
    
	FATFS_getfree(fs, path, &tot_sect, &fre_sect);
    f_res = f_open(&file, test_file_path, FA_CREATE_ALWAYS|FA_WRITE|FA_READ);
    
    if(f_res != FR_OK)
    {
        __msg("FATFS open file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS open file secc\r\n");
	
    /* 格式化写入，参数格式类似printf函数 */
    f_printf(&file,"-----------Total:%10lu KB\nAvailable:  %10lu KB\n", tot_sect, fre_sect);
    
    /*  文件定位到文件起始位置 */
    f_res = f_lseek(&file,0);
    
    if(f_res != FR_OK)
    {
        __msg("FATFS lseek file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS lseek file secc\r\n");
    
    f_printf(&file,"line head\n");
    
    /*  文件定位到行尾  */
    f_res = f_lseek(&file,f_size(&file)-1);
    if(f_res != FR_OK)
    {
        __msg("FATFS lseek file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    __msg("FATFS lseek file secc\r\n");
    
    f_printf(&file,"line tail\n");
    
    /* 读取文件所有内容到缓存区 */
    f_res = f_read(&file,readbuffer, f_size(&file), &fnum);
    if(f_res != FR_OK)
    {
        __msg("FATFS read file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS read file secc\r\n");
    
    __msg("file content:\n%s\n",readbuffer);
    
    f_res = f_close(&file); 
    if(f_res != FR_OK)
    {
        __msg("FATFS close file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS close file secc\r\n");
 
    __msg("FATFS f_lseek and f_printf test end\r\n");
    __msg("////////////////////////////////////\r\n"); 
    return 1;
}

/**
  * 函数功能: 文件系统目录创建和重命名功能测试
  * 输入参数: 无
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_opendir_test(const char *path)
{
	FRESULT f_res; 
    DIR dir;
    const char test_dir[] = "TestDir";
	const char test_file[] = "test2.txt";
    char test_dir_path[20] = {0};
	char test_file1_path[40] = {0};
	char test_file2_path[20] = {0};
    
	sprintf(test_dir_path, "%s%s", path, test_dir);
	sprintf(test_file1_path, "%s%s//testdir.txt", path, test_dir);
	sprintf(test_file2_path, "%s%s", path, test_file);
    /*  目录创建和重命名功能测试  */
    __msg("////////////////////////////////////\r\n"); 
    __msg("FATFS dir create and rename test start\r\n");
    /* 尝试打开目录 */
    f_res=f_opendir(&dir, test_dir_path);
    if(f_res != FR_OK)
    {
        __msg("FATFS opendir fail\r\n");
        printf_fatfs_error(f_res);
        __msg("FATFS create dir\r\n");
        /* 打开目录失败，就创建目录 */
        f_res = f_mkdir(test_dir_path);
        if(f_res != FR_OK)
        {
            __msg("FATFS mkdir fail\r\n");
            printf_fatfs_error(f_res);
            return 0;
        }
        __msg("FATFS mkdir secc\r\n");
    }
    else
    {
        /* 如果目录已经存在，关闭它 */
        f_res = f_closedir(&dir);
        if(f_res != FR_OK)
        {
            __msg("FATFS close dir fail\r\n");
            printf_fatfs_error(f_res);
            return 0;
        }
        __msg("FATFS close dir secc\r\n");
        
        /* 删除文件 */
        f_res = f_unlink(test_file1_path);
        if(f_res != FR_OK)
        {
            __msg("FATFS unlink file fail\r\n");
            printf_fatfs_error(f_res);
        }
        __msg("FATFS unlink file secc\r\n");
    }
    
    /* 重命名并移动文件 */
    f_res = f_rename(test_file2_path, test_file1_path);      
    if(f_res != FR_OK)
    {
        __msg("FATFS rename file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    __msg("FATFS rename file secc\r\n");
    
    __msg("FATFS dir create and rename test end\r\n");
    __msg("////////////////////////////////////\r\n"); 
    return 1;
}

uint32_t FATFS_getfree(FATFS *fs, const char *path, uint32_t *tot_sect, uint32_t *fre_sect)
{
	FATFS *fls = fs;
	DWORD fre_clust;
	FRESULT f_res; 
	
	/* 获取设备信息和空簇大小 */
    f_res = f_getfree(path, &fre_clust, &fls);
    if(f_res != FR_OK)
    {
        __msg("FATFS getfree file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	__msg("FATFS getfree file secc\r\n");
    
    /* 计算得到总的扇区个数和空扇区个数 */
    *tot_sect = (fls->n_fatent - 2) * fls->csize;
    *fre_sect = fre_clust * fls->csize;
	*tot_sect *= 4;
	*fre_sect *= 4;
	
	__msg("FATFS write: Total:%10lu KB\nAvailable:  %10lu KB\r\n", *tot_sect, *fre_sect);
	
	return 1;
}

/**
  * 函数功能: 递归扫描FatFs内的文件
  * 输入参数: 初始扫描路径 此空间要足够大 函数会往数组里面增加内容
  * 返 回 值: 1
  * 说    明: 无
  */
uint32_t FATFS_scan_disk(const char* disk_path) 
{
	uint8_t path[100] = {0};
  
    memcpy(path, disk_path, strlen(disk_path));
	
    __msg("////////////////////////////////////\r\n"); 
    __msg("FATFS scan files path == %s start\r\n", path); 
    FATFS_scan_files(path);
    __msg("FATFS scan files path == %s end\r\n", path); 
    __msg("////////////////////////////////////\r\n"); 
    
    return 1;
}

/**
  * 函数功能: 递归扫描FatFs内的文件
  * 输入参数: 初始扫描路径 此空间要足够大 函数会往数组里面增加内容
  * 返 回 值: 文件系统的返回值
  * 说    明: 无
  */
FRESULT FATFS_scan_files(char* path) 
{ 
    DIR dir;
    FRESULT res;
    FILINFO fno;  
    int i;            
    char *fn;        // 文件名

    //打开目录
    res = f_opendir(&dir, path); 
    if (res != FR_OK) 
    { 
		printf_fatfs_error(res);
        return res; 
    }
   
    i = strlen(path); 
    for (;;) 
	{ 
        //读取目录下的内容，再读会自动读下一个文件
        res = f_readdir(&dir, &fno); 								
        //为空时表示所有项目读取完毕，跳出
        if (res != FR_OK || fno.fname[0] == 0) break; 	
        
        fn = fno.fname;
        //点表示当前目录，跳过			
        if (*fn == '.') continue; 	
        //目录，递归读取      
        if (fno.fattrib & AM_DIR)         
		{ 			
             //合成完整目录名        
             sprintf(&path[i], "/%s", fn); 		
             //递归遍历         
             res = FATFS_scan_files(path);	
             path[i] = 0;         
             //打开失败，跳出循环        
             if (res != FR_OK) 
             break; 
        } 
		else 
		{ 
            __msg("%s/%s\n", path, fn);
            //输出文件名	
            /*  可以在这里提取特定格式的文件路径  */        
        }//else
    }//for
 
    
    return res; 
}

/**
  * 函数功能: 将disk格式化为FatFS文件系统
  * 输入参数: FatFS文件系统 FatFS文件系统路径
  * 返 回 值: 操作结果
  * 说    明: 无
  */
FRESULT FATFS_format_disk(FATFS *fs, const TCHAR *path)
{
	FRESULT res;
	uint8_t work_buff[4096] = {0};

	__msg("format disk\n");
	res = f_mkfs(path, 1, 4096, work_buff, 4096);

    if (res != FR_OK)
    {
        __msg("format disk fail\r\n");
        printf_fatfs_error(res);
        return res;
    }
    __msg("format_disk OK!\n");    
    
    /*  重新挂载  */
    __msg("mount afresh\n");
    __msg("unmount\n");
    res = f_mount(0, path, 0);
    if(res != FR_OK)
    {
        __msg("unmount disk fail\r\n");
        printf_fatfs_error(res);
        return res;
    }
    
    __msg("mount\n");
    res = f_mount(fs, path, 0);
    if(res != FR_OK)
    {
        __msg("mount fail\r\n");
        printf_fatfs_error(res);
        return res;
    }
    __msg("format disk and mount afresh secc\r\n");
    return res;
}

/**
  * 函数功能: FatFS文件系统操作结果信息处理.
  * 输入参数: FatFS文件系统操作结果：FRESULT
  * 返 回 值: 无
  * 说    明: 无
  */
void printf_fatfs_error(FRESULT fresult)
{
  switch(fresult)
  {
    case FR_OK:                   //(0)
      __msg("Succeeded\r\n");
    break;
    case FR_DISK_ERR:             //(1)
      __msg("A hard error occurred in the low level disk I/O layer\r\n");
    break;
    case FR_INT_ERR:              //(2)
      __msg("Assertion failed\r\n");
    break;
    case FR_NOT_READY:            //(3)
      __msg("The physical drive cannot work\r\n");
    break;
    case FR_NO_FILE:              //(4)
      __msg("Could not find the file\r\n");
    break;
    case FR_NO_PATH:              //(5)
      __msg("Could not find the path\r\n");
    break;
    case FR_INVALID_NAME:         //(6)
      __msg("The path name format is invalid\r\n");
    break;
    case FR_DENIED:               //(7)
    case FR_EXIST:                //(8)
      __msg("Access denied due to prohibited access\r\n");
    break;
    case FR_INVALID_OBJECT:       //(9)
      __msg("The file/directory object is invalid\r\n");
    break;
    case FR_WRITE_PROTECTED:      //(10)
      __msg("The physical drive is write protected\r\n");
    break;
    case FR_INVALID_DRIVE:        //(11)
      __msg("The logical drive number is invalid\r\n");
    break;
    case FR_NOT_ENABLED:          //(12)
      __msg("The volume has no work area\r\n");
    break;
    case FR_NO_FILESYSTEM:        //(13)
      __msg("There is no valid FAT volume\r\n");
    break;
    case FR_MKFS_ABORTED:         //(14)
      __msg("The f_mkfs() aborted due to any parameter error\r\n");
    break;
    case FR_TIMEOUT:              //(15)
      __msg("Could not get a grant to access the volume within defined period\r\n");
    break;
    case FR_LOCKED:               //(16)
      __msg("The operation is rejected according to the file sharing policy\r\n");
    break;
    case FR_NOT_ENOUGH_CORE:      //(17)
      __msg("LFN working buffer could not be allocated\r\n");
    break;
    case FR_TOO_MANY_OPEN_FILES:  //(18)
      __msg("Number of open files > _FS_SHARE\r\n");
    break;
    case FR_INVALID_PARAMETER:    // (19)
      __msg("Given parameter is invalid\r\n");
    break;
    default:break;
  }
}
