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
    uint8_t path[100] = {0};
  
    memcpy(path, disk_path, strlen(disk_path));
  
	if(FATFS_mount(fs, disk_path) != 1)return 0;
        
	if(FATFS_write_read_test() != 1)return 0;
	
    if(FATFS_seek_printf_test(fs) != 1)return 0;

    if(FATFS_opendir_test() != 1)return 0;
    
    FATFS_scan_disk(path);
    
    return 1;
    /*  fat write read test end  */
}

/**
  * 函数功能: 文件系统挂载
  * 输入参数: FATFS文件系统 path文件系统路径
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_mount(FATFS *fs, const TCHAR *path)
{
    FRESULT f_res; 
    /*  fat mount  */
    printf("////////////////////////////////////\r\n"); 
    printf("FATFS mount start\r\n"); 
	f_res = f_mount(fs, path, 0);
	
    /*  没有文件系统 开始格式化  */
    if(f_res != FR_NO_FILESYSTEM)
    {
        f_res = fatfs_format_disk(fs, path);
        
        if(f_res != FR_OK)
        {
            printf_fatfs_error(f_res); 
            return 0;
        }
    }
    else if(f_res == FR_OK)
    {
        printf("FATFS mount secc\r\n"); 
    }
    else
    {
        printf_fatfs_error(f_res);
        return 0;
    }
    
    printf("FATFS mount end\r\n"); 
    printf("////////////////////////////////////\r\n"); 
    return 1;
}

/**
  * 函数功能: 文件读写测试
  * 输入参数: 无
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_write_read_test(void)
{
    FIL file;
	FRESULT f_res; 
	UINT fnum;
	
    const TCHAR file_path[] = "0:/test.txt";
	uint8_t file_write_buff[FILE_TEST_BUF_NUM] = "0123456789abcdef";
	uint8_t file_read_buff[FILE_TEST_BUF_NUM] = {0};
    
    printf("////////////////////////////////////\r\n"); 
    printf("FATFS write read test start\r\n");
	f_res = f_open(&file, file_path,FA_CREATE_ALWAYS | FA_WRITE );
    
    if(f_res != FR_OK)
    {
        printf("FATFS open file fail\r\n");
        printf_fatfs_error(f_res);
        
        return 0;
    }
    
    printf("FATFS open create file secc\r\n");
    
    f_res = f_write(&file, file_write_buff, FILE_TEST_BUF_NUM, &fnum);
    
    if(f_res != FR_OK)
    {
        printf("FATFS write file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    printf("FATFS write file secc\r\n");
    printf("write num == %d\r\n", FILE_TEST_BUF_NUM);
    printf("write secc num == %d\r\n", fnum);
    
	f_res = f_close(&file);
    
    if(f_res != FR_OK)
    {
        printf("FATFS close file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    printf("FATFS close file secc\r\n");
    
    /*  fat read test  */
	printf("FATFS read test\r\n");
	f_res = f_open(&file, file_path, FA_OPEN_EXISTING | FA_READ);
    
    if(f_res != FR_OK)
    {
        printf("FATFS open file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
   
    printf("FATFS open file secc\r\n");
    f_res = f_read(&file, file_read_buff, FILE_TEST_BUF_NUM, &fnum); 
    
    if(f_res != FR_OK)
    {
        printf("FATFS read file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    printf("FATFS read file secc\r\n");
    printf("read num == %d\r\n", FILE_TEST_BUF_NUM);
	printf("read secc num == %d\r\n", fnum);
    
	f_res = f_close(&file);
    
	if(f_res != FR_OK)
    {
        printf("FATFS close file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    
    printf("FATFS close file secc\r\n");
    printf("FATFS write read test end\r\n");
    printf("////////////////////////////////////\r\n"); 
    return 1;
}

/**
  * 函数功能: 文件定位和格式化写入功能测试
  * 输入参数: FatFS文件系统
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_seek_printf_test(FATFS *fs)
{
    FIL file;
    FATFS *fls = fs;
	FRESULT f_res; 
	UINT fnum;
    DWORD fre_clust;
    DWORD tot_sect;
    DWORD fre_sect;
    uint8_t readbuffer[512] = {0};
    const TCHAR file_path[] = "0:/test2.txt";
    
    /*  文件定位和格式化写入功能测试  */
    printf("////////////////////////////////////\r\n"); 
    printf("FATFS f_lseek and f_printf test start\r\n");
    
    /* 获取设备信息和空簇大小 */
    f_res = f_getfree((const TCHAR *)"/", &fre_clust, &fls);
    if(f_res != FR_OK)
    {
        printf("FATFS getfree file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	printf("FATFS getfree file secc\r\n");
    
    /* 计算得到总的扇区个数和空扇区个数 */
    tot_sect = (fls->n_fatent - 2) * fls->csize;
    fre_sect = fre_clust * fls->csize;
    
    f_res = f_open(&file, file_path, FA_CREATE_ALWAYS|FA_WRITE|FA_READ);
    
    if(f_res != FR_OK)
    {
        printf("FATFS open file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	printf("FATFS open file secc\r\n");
    
	
    /* 格式化写入，参数格式类似printf函数 */
    
    f_printf(&file,"-----------Total:%10lu KB\nAvailable:  %10lu KB\n", tot_sect *4, fre_sect *4);
    
    /*  文件定位到文件起始位置 */
    f_res = f_lseek(&file,0);
    
    if(f_res != FR_OK)
    {
        printf("FATFS lseek file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	printf("FATFS lseek file secc\r\n");
    
    f_printf(&file,"line head\n");
    
    /*  文件定位到行尾  */
    f_res = f_lseek(&file,f_size(&file)-1);
    if(f_res != FR_OK)
    {
        printf("FATFS lseek file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    printf("FATFS lseek file secc\r\n");
    
    f_printf(&file,"line tail\n");
    
    /* 读取文件所有内容到缓存区 */
    f_res = f_read(&file,readbuffer, f_size(&file), &fnum);
    if(f_res != FR_OK)
    {
        printf("FATFS read file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	printf("FATFS read file secc\r\n");
    
    printf("file content:\n%s\n",readbuffer);
    
    f_res = f_close(&file); 
    if(f_res != FR_OK)
    {
        printf("FATFS close file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
	printf("FATFS close file secc\r\n");
 
    printf("FATFS f_lseek and f_printf test end\r\n");
    printf("////////////////////////////////////\r\n"); 
    return 1;
}

/**
  * 函数功能: 文件系统目录创建和重命名功能测试
  * 输入参数: 无
  * 返 回 值: fail:0 secc:1
  * 说    明: 无
  */
uint32_t FATFS_opendir_test(void)
{
	FRESULT f_res; 
    DIR dir;
    //const TCHAR dir_path[] = "0:/TestDir";
    const TCHAR dir_path[] = "0:/TestDir";
    
    /*  目录创建和重命名功能测试  */
    printf("////////////////////////////////////\r\n"); 
    printf("FATFS dir create and rename test start\r\n");
    /* 尝试打开目录 */
    f_res=f_opendir(&dir, dir_path);
    if(f_res != FR_OK)
    {
        printf("FATFS opendir fail\r\n");
        printf_fatfs_error(f_res);
        printf("FATFS create dir\r\n");
        /* 打开目录失败，就创建目录 */
        f_res = f_mkdir(dir_path);
        if(f_res != FR_OK)
        {
            printf("FATFS mkdir fail\r\n");
            printf_fatfs_error(f_res);
            return 0;
        }
        printf("FATFS mkdir secc\r\n");
    }
    else
    {
        /* 如果目录已经存在，关闭它 */
        f_res = f_closedir(&dir);
        if(f_res != FR_OK)
        {
            printf("FATFS close dir fail\r\n");
            printf_fatfs_error(f_res);
            return 0;
        }
        printf("FATFS close dir secc\r\n");
        
        /* 删除文件 */
        f_res = f_unlink("TestDir/testdir.txt");
        if(f_res != FR_OK)
        {
            printf("FATFS unlink file fail\r\n");
            printf_fatfs_error(f_res);
            return 0;
        }
        printf("FATFS unlink file secc\r\n");
    }
    
    /* 重命名并移动文件 */
    f_res = f_rename("test2.txt","TestDir/testdir.txt");      
    if(f_res != FR_OK)
    {
        printf("FATFS rename file fail\r\n");
        printf_fatfs_error(f_res);
        return 0;
    }
    printf("FATFS rename file secc\r\n");
    
    printf("FATFS dir create and rename test end\r\n");
    printf("////////////////////////////////////\r\n"); 
    return 1;
}

/**
  * 函数功能: 递归扫描FatFs内的文件
  * 输入参数: 初始扫描路径 此空间要足够大 函数会往数组里面增加内容
  * 返 回 值: 1
  * 说    明: 无
  */
uint32_t FATFS_scan_disk(char* path) 
{
    printf("////////////////////////////////////\r\n"); 
    printf("FATFS scan files start\r\n"); 
    FATFS_scan_files(path);
    printf("FATFS scan files end\r\n"); 
    printf("////////////////////////////////////\r\n"); 
    
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
            printf("%s/%s\n", path, fn);
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
FRESULT fatfs_format_disk(FATFS *fs, const TCHAR *path)
{
	FRESULT res;
	uint8_t work_buff[4096] = {0};

	printf("format disk\n");
	res = f_mkfs(path, 1, 4096, work_buff, 4096);

    if (res != FR_OK)
    {
        printf("format disk fail\r\n");
        printf_fatfs_error(res);
        return res;
    }
    printf("format_disk OK!\n");    
    
    /*  重新挂载  */
    printf("mount afresh\n");
    printf("unmount\n");
    res = f_mount(0, path, 0);
    if(res != FR_OK)
    {
        printf("unmount disk fail\r\n");
        printf_fatfs_error(res);
        return res;
    }
    
    printf("mount\n");
    res = f_mount(fs, path, 0);
    if(res != FR_OK)
    {
        printf("mount fail\r\n");
        printf_fatfs_error(res);
        return res;
    }
    printf("format disk and mount afresh secc\r\n");
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
      printf("Succeeded\r\n");
    break;
    case FR_DISK_ERR:             //(1)
      printf("A hard error occurred in the low level disk I/O layer\r\n");
    break;
    case FR_INT_ERR:              //(2)
      printf("Assertion failed\r\n");
    break;
    case FR_NOT_READY:            //(3)
      printf("The physical drive cannot work\r\n");
    break;
    case FR_NO_FILE:              //(4)
      printf("Could not find the file\r\n");
    break;
    case FR_NO_PATH:              //(5)
      printf("Could not find the path\r\n");
    break;
    case FR_INVALID_NAME:         //(6)
      printf("The path name format is invalid\r\n");
    break;
    case FR_DENIED:               //(7)
    case FR_EXIST:                //(8)
      printf("Access denied due to prohibited access\r\n");
    break;
    case FR_INVALID_OBJECT:       //(9)
      printf("The file/directory object is invalid\r\n");
    break;
    case FR_WRITE_PROTECTED:      //(10)
      printf("The physical drive is write protected\r\n");
    break;
    case FR_INVALID_DRIVE:        //(11)
      printf("The logical drive number is invalid\r\n");
    break;
    case FR_NOT_ENABLED:          //(12)
      printf("The volume has no work area\r\n");
    break;
    case FR_NO_FILESYSTEM:        //(13)
      printf("There is no valid FAT volume\r\n");
    break;
    case FR_MKFS_ABORTED:         //(14)
      printf("The f_mkfs() aborted due to any parameter error\r\n");
    break;
    case FR_TIMEOUT:              //(15)
      printf("Could not get a grant to access the volume within defined period\r\n");
    break;
    case FR_LOCKED:               //(16)
      printf("The operation is rejected according to the file sharing policy\r\n");
    break;
    case FR_NOT_ENOUGH_CORE:      //(17)
      printf("LFN working buffer could not be allocated\r\n");
    break;
    case FR_TOO_MANY_OPEN_FILES:  //(18)
      printf("Number of open files > _FS_SHARE\r\n");
    break;
    case FR_INVALID_PARAMETER:    // (19)
      printf("Given parameter is invalid\r\n");
    break;
    default:break;
  }
}
