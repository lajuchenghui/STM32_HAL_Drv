/**********************************************************************************************************
*	函 数 名: byte_buf_seek
*	功能说明: 从一段byte数组buf1中寻找另一个byte数组buf2的位置
*	传    参: buf1：大数组 buf2：小数组 len1：buf1的长度 len2：buf2的长度
*	返 回 值: buf2在buf1里的位置 -1：没找到 -2：参数错误
*	说    明: 
*********************************************************************************************************/
int32_t byte_buf_seek(int8_t *buf1, int8_t *buf2, int32_t len1, int32_t len2)
{
	int32_t i;
	int32_t loc = 0;
	
	if(buf1 == 0)return -2;
	if(buf2 == 0)return -2;
	if(len1 < 1)return -2;
	if(len2 < 1)return -2;
	if(len1 < len2)return -2;
	
	for(i=0;i<len1;i++)
	{
		if(buf1[i] == buf2[loc])
		{
			loc++;
			if(loc == len2)
			{
				return i-len2 + 1;
			}
		}	
		else
		{
			if(loc != 0)
			{
				return -1;
			}
		}
	}
	/*  head seek fail  */
	__wrn("head seek fail\r\n");
	return -1;
}
/**********************************************************************************************************
*	函 数 名: byte_buf_xor_sum
*	功能说明: 从一段byte数组buf进行异或校验
*	传    参: buf：数组 len：要校验的长度
*	返 回 值: 校验的值 -1：参数错误
*	说    明: 
*********************************************************************************************************/
int32_t byte_buf_xor_sum(int8_t *buf, uint32_t len)
{
	int32_t i;
	int32_t tmp = 0;
	
	if(buf == 0)return -1;
	if(len < 2)return -1;
	for(tmp=buf[0],i=0;i<len-1;i++)
	{
	  tmp ^= buf[i + 1];
	}
	
	return tmp;
}
