/********************************************************************************
	* Function List:
	*	1. void I2C_Configuration(void) -- 配置CPU的硬件I2C
	* 2. void I2C_WriteByte(uint8_t addr,uint8_t data) -- 向寄存器地址写一个byte的数据
	* 3. void WriteCmd(unsigned char I2C_Command) -- 写命令
	* 4. void WriteDat(unsigned char I2C_Data) -- 写数据
	* 5. void OLED_Init(void) -- OLED屏初始化
	* 6. void OLED_SetPos(unsigned char x, unsigned char y) -- 设置起始点坐标
	* 7. void OLED_Fill(unsigned char fill_Data) -- 全屏填充
	* 8. void OLED_CLS(void) -- 清屏
	* 9. void OLED_ON(void) -- 唤醒
	* 10. void OLED_OFF(void) -- 睡眠
	* 11. void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- 显示字符串(字体大小有6*8和8*16两种)
	* 12. void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N) -- 显示中文(中文需要先取模，然后放到codetab.h中)
	* 13. void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMP图片
	*
  *
  ******************************************************************************
  */ 




#include "OLED_I2C.h"
#include "sw_delay.h"
#include "codetab.h"




//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 
uint8_t OLED_GRAM[128][8];


//这两个参数定义动画每帧的长宽。
#define MOVIE_XSIZE	100 
#define MOVIE_YSIZE 40  
#define FRAME_SIZE MOVIE_XSIZE*MOVIE_YSIZE/8   
							  
//x:0~127
//y:0~63
//picx:图片编号		 
void Movie_Show_Img(uint16_t x,uint16_t y,uint32_t picx)
{      			    
	uint16_t temp,t,t1;
	uint16_t y0=y;	
	picx*=FRAME_SIZE;				    				   
    for(t=0;t<FRAME_SIZE;t++)
    {   
		temp=movie_data[picx+t];  //得到图片的一个字节数据	                          
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80) OLED_DrawPoint(x,y,1); 
			else OLED_DrawPoint(x,y,0);		   
			temp<<=1;
			y++;
			if((y-y0)==MOVIE_YSIZE)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }          
}
 

void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t pos,bx,temp=0;
	if(x>127||y>63)return;//超出范围了.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;
	else OLED_GRAM[x][pos]&=~temp;	    
}


//更新显存到LCD		 
void OLED_Refresh_Gram(void)
{
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		WriteCmd(0xb0+i);    //设置页地址（0~7）
        WriteCmd(0x00);      //设置显示位置—列低地址
        WriteCmd(0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++) 
        {
            WriteDat(OLED_GRAM[n][i]); 
        } 
	}   
}




 /**
  * @brief  I2C_Configuration，初始化硬件IIC引脚
  * @param  无
  * @retval 无
  */
void I2C_Configuration(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure; 

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	/*STM32F103C8T6芯片的硬件I2C: PB6 -- SCL; PB7 -- SDA */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//I2C必须开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_DeInit(I2C1);//使用I2C1
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x30;//主机的I2C地址,随便写的
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;//400K

	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
}


 /**
  * @brief  I2C_WriteByte，向OLED寄存器地址写一个byte的数据
  * @param  addr：寄存器地址
	*					data：要写入的数据
  * @retval 无
  */
void I2C_WriteByte(uint8_t addr,uint8_t data)
{
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C1, ENABLE);//开启I2C1
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

	I2C_Send7bitAddress(I2C1, OLED_ADDRESS, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C1, addr);//寄存器地址
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(I2C1, data);//发送数据
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP(I2C1, ENABLE);//关闭I2C1总线
}


 /**
  * @brief  WriteCmd，向OLED写入命令
  * @param  I2C_Command：命令代码
  * @retval 无
  */
void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}


 /**
  * @brief  WriteDat，向OLED写入数据
  * @param  I2C_Data：数据
  * @retval 无
  */
void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}


 /**
  * @brief  OLED_Init，初始化OLED
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	sw_delay_ms(100); //这里的延时很重要
	
#ifdef SDCARDTEST
    
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel

#elif defined OLED_MOVIE
    WriteCmd(0xAE); //关闭显示
	WriteCmd(0xD5); //设置时钟分频因子,震荡频率
	WriteCmd(80);   //[3:0],分频因子;[7:4],震荡频率
	WriteCmd(0xA8); //设置驱动路数
	WriteCmd(0X3F); //默认0X3F(1/64) 
	WriteCmd(0xD3); //设置显示偏移
	WriteCmd(0X00); //默认为0

	WriteCmd(0x40); //设置显示开始行 [5:0],行数.
													    
	WriteCmd(0x8D); //电荷泵设置
	WriteCmd(0x14); //bit2，开启/关闭
	WriteCmd(0x20); //设置内存地址模式
	WriteCmd(0x02); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
	WriteCmd(0xA1); //段重定义设置,bit0:0,0->0;1,0->127;
	WriteCmd(0xC0); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
	WriteCmd(0xDA); //设置COM硬件引脚配置
	WriteCmd(0x12); //[5:4]配置
		 
	WriteCmd(0x81); //对比度设置
	WriteCmd(0xEF); //1~255;默认0X7F (亮度设置,越大越亮)
	WriteCmd(0xD9); //设置预充电周期
	WriteCmd(0xf1); //[3:0],PHASE 1;[7:4],PHASE 2;
	WriteCmd(0xDB); //设置VCOMH 电压倍率
	WriteCmd(0x30); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	WriteCmd(0xA4); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	WriteCmd(0xA6); //设置显示方式;bit0:1,反相显示;0,正常显示	    						   
	WriteCmd(0xAF); //开启显示	
#endif

}


 /**
  * @brief  OLED_SetPos，设置光标
  * @param  x,光标x位置
	*					y，光标y位置
  * @retval 无
  */
void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

 /**
  * @brief  OLED_Fill，填充整个屏幕
  * @param  fill_Data:要填充的数据
	* @retval 无
  */
void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}

 /**
  * @brief  OLED_CLS，清屏
  * @param  无
	* @retval 无
  */
void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}


 /**
  * @brief  OLED_ON，将OLED从休眠中唤醒
  * @param  无
	* @retval 无
  */
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}


 /**
  * @brief  OLED_OFF，让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
  * @param  无
	* @retval 无
  */
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}


 /**
  * @brief  OLED_ShowStr，显示codetab.h中的ASCII字符,有6*8和8*16可选择
  * @param  x,y : 起始点坐标(x:0~127, y:0~7);
	*					ch[] :- 要显示的字符串; 
	*					TextSize : 字符大小(1:6*8 ; 2:8*16)
	* @retval 无
  */
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

 /**
  * @brief  OLED_ShowCN，显示codetab.h中的汉字,16*16点阵
  * @param  x,y: 起始点坐标(x:0~127, y:0~7); 
	*					N:汉字在codetab.h中的索引
	* @retval 无
  */
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
}



 /**
  * @brief  OLED_DrawBMP，显示BMP位图
  * @param  x0,y0 :起始点坐标(x0:0~127, y0:0~7);
	*					x1,y1 : 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
	* @retval 无
  */
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}



void OLED_ShowInterger(unsigned long x, unsigned char* result)
{
    result[0] = x/1000 + 0x30;                 // High
    result[1] = x%1000/100 + 0x30; 
    result[2] = x%100/10 + 0x30; 
    result[3] = x%10 + 0x30; 
}


