/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
FATFS fs;         /* Work area (file system object) for logical drive */
FIL fsrc, fdst;      /* file objects */
FRESULT res;
UINT br,bw;
char path0[512]="0:";
char buffer[4096];   /* file copy buffer */
uint8_t textFileBuffer[] = "��Ӣ�Ĳ����ַ��� \r\nChinese and English test strings \r\n";

uint8_t buffer_itoa[4] = {0};
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
  * @��������	Delay
  * @����˵��   ʵ����ʱ���� 
  * @�������   nCount: ��ʱʱ���������ֵԽ����ʱʱ��Խ�� 
  * @�������   ��
  * @���ز���   ��
  * @ע������	��
  *****************************************************************************/
void  Delay (uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}
/*******************************************************************************
  * @��������	scan_files
  * @����˵��   �����ļ�Ŀ¼�������ļ� 
  * @�������   path: ��Ŀ¼ 
  * @�������   ��
  * @���ز���   FRESULT
  * @ע������	��
  *****************************************************************************/
FRESULT scan_files (
    char* path        /* Start node to be scanned (also used as work area) */
)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    static char lfn[_MAX_LFN + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                sprintf(&path[i], "/%s", fn);
				printf("scan file - %s\n\r",path);
                
                OLED_ShowStr(0,2,(unsigned char*)fn,1); 
                OLED_ShowStr(0,2,(unsigned char*)path,1);
                               
                res = scan_files(path);
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("scan file - %s/%s\n\r", path, fn);
                OLED_ShowStr(0,3,(unsigned char*)fn,1);
                OLED_ShowStr(0,3,(unsigned char*)path,1);  
            }
        }
    }else{
		printf("scan files error : %d\n\r",res);
	}

    return res;
}
/*******************************************************************************
  * @��������	SD_TotalSize
  * @����˵��   �ļ��ռ�ռ����� 
  * @�������   �� 
  * @�������   ��
  * @���ز���   1: �ɹ� 
  				0: ʧ��
  * @ע������	��
  *****************************************************************************/
int SD_TotalSize(char *path)
{
    FATFS *fs;
    DWORD fre_clust;        

    res = f_getfree(path, &fre_clust, &fs);  /* �����Ǹ�Ŀ¼��ѡ�����0 */
    if ( res==FR_OK ) 
    {
	  printf("\n\rget %s drive space.\n\r",path);
	  /* Print free space in unit of MB (assuming 512 bytes/sector) */
      printf("%d MB total drive space.\r\n"
           "%d MB available.\r\n",
           ( (fs->n_fatent - 2) * fs->csize ) / 2 /1024 , (fre_clust * fs->csize) / 2 /1024 );
        
      OLED_ShowStr(0,5,"get ",1); 
      OLED_ShowStr(24,5,(unsigned char*)path,1);
      OLED_ShowStr(36,5,"drive space",1);  
        
      OLED_ShowInterger((((fs->n_fatent - 2) * fs->csize ) / 2 /1024), buffer_itoa);  
      OLED_ShowStr(0,6,buffer_itoa,1);  
      OLED_ShowStr(24,6,"MB total space",1);
        
      OLED_ShowInterger(((fre_clust * fs->csize) / 2 /1024), buffer_itoa);   
      OLED_ShowStr(0,7,buffer_itoa,1);  
      OLED_ShowStr(24,7,"MB availble space",1);  
	  return 1;
	}
	else
	{ 
	  printf("\n\rGet total drive space faild!\n\r");
	  return 0;   
	}
}

/**
  * @brief  ���ڴ�ӡ���
  * @param  None
  * @retval None
  */
int main(void)
{
    SystemInit();
	USART_Configuration();
    LED_Init();
    
	I2C_Configuration();
	OLED_Init();
    OLED_Fill(0x00);
    OLED_ShowStr(0,0,(unsigned char*)"  USB EVK Board V0.1",1);
        
	printf("\r\n*******************************************************************************");
	printf("\r\n*************************** SPI->SD Card->FATFS *******************************");
	printf("\r\n*******************************************************************************");
	printf("\r\n");
	//����SPI1�ӿ�
	MSD1_SPI_Configuration();
	//�����ļ�ϵͳ
	res = f_mount(0,&fs);
	if(res != FR_OK){
		printf("mount filesystem 0 failed : %d\n\r",res);
	}
	//д�ļ�����
	printf("write file test......\n\r");
    res = f_open(&fdst, "0:/longfilenametest.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if(res != FR_OK){
		printf("open file error : %d\n\r",res);
	}else{
	    res = f_write(&fdst, textFileBuffer, sizeof(textFileBuffer), &bw);               /* Write it to the dst file */
		if(res == FR_OK){
			printf("write data ok! %d\n\r",bw);
		}else{
			printf("write data error : %d\n\r",res);
		}
		/*close file */
		f_close(&fdst);
	}

	//���ļ�����
	printf("read file test......\n\r");
    res = f_open(&fsrc, "0:/longfilenametest.txt", FA_OPEN_EXISTING | FA_READ);
    if(res != FR_OK){
		printf("open file error : %d\n\r",res);
	}else{
	    res = f_read(&fsrc, buffer, sizeof(textFileBuffer), &br);     /* Read a chunk of src file */
		if(res==FR_OK){
			printf("read data num : %d\n\r",br);
			printf("%s\n\r",buffer);
		}else{
			printf("read file error : %d\n\r",res);
		}
		/*close file */
		f_close(&fsrc);
	}
	//ɨ���Ѿ����ڵ��ļ�
	printf("\n\rbegin scan files path0......\n\r");
	scan_files(path0);

	SD_TotalSize(path0);//��ȡSD����
	while(1)
	{
        LEDx_OnOff(GPIOC, GPIO_Pin_13, 0);
        sw_delay_ms(300);
        LEDx_OnOff(GPIOC, GPIO_Pin_13, 1);
        
        LEDx_OnOff(GPIOC, GPIO_Pin_14, 0);
        sw_delay_ms(300);
        LEDx_OnOff(GPIOC, GPIO_Pin_14, 1);
        
        LEDx_OnOff(GPIOC, GPIO_Pin_15, 0);
        sw_delay_ms(300);
        LEDx_OnOff(GPIOC, GPIO_Pin_15, 1);	
        
#ifdef OLED_MOVIE
        
        Movie_Show_Img(14,12,t);//ͼƬ��ʾ��������
        OLED_Refresh_Gram();
		if(t<12)        sw_delay_ms(600);
		else if(t<14)   sw_delay_ms(1000);	  
		else if(t<15)   sw_delay_ms(1800);
		else if(t<19)   sw_delay_ms(800);
		else if(t<20)   sw_delay_ms(1800);
		else if(t<130)  sw_delay_ms(100);
		else if(t<132)
		{
			sw_delay_ms(1500);
			sw_delay_ms(1500);
		}else if(t<133) sw_delay_ms(1000);
		else if(t<140)  sw_delay_ms(100);
		else if(t<141)
		{
			sw_delay_ms(1800);
			sw_delay_ms(1200);
		} 
		t++;			  
		if(t>140)t=0; 
#endif
	}
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  �����ڼ�������������ʱ��Դ�ļ����ʹ�������
  * @param  file Դ�ļ���
  * @param  line ������������ 
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* �û����������Լ��Ĵ������ڱ��������ļ�������������,
       ���磺printf("�������ֵ: �ļ��� %s �� %d��\r\n", file, line) */

    /* ����ѭ�� */
    while (1)
    {
    }
}
#endif

/*********************************END OF FILE**********************************/
