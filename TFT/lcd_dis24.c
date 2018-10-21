/****************************************************************************
* Copyright (C), 2011 �ܶ�Ƕ��ʽ������ www.ourstm.net
*
* �������� �ܶ���STM32������V2,2.1,V3,MINI�ϵ���ͨ��           
* QQ: 9191274, ������sun68, Email: sun68@163.com 
* �Ա����̣�ourstm.taobao.com  
*
* �ļ���: Fun.c
* ���ݼ���:
*       �������ṩ�˷ܶ�����4.3����ģ�����������
		
*
* �ļ���ʷ:
* �汾��  ����       ����    ˵��
* v0.2    2011-07-18 sun68  �������ļ�
*
*/

#include "fsmc_sram.h"
#include "lcdconf.h"



void LCD_Init1(void);
void LCD_WR_REG(unsigned int index);
void LCD_WR_CMD(unsigned int index,unsigned int val);
unsigned short LCD_RD_data(void);
void LCD_WR_Data(unsigned int val);	
void LCD_clear(void);

//=================================================================================

//�ߴ�������ʾ����

//=================================================================================
/*
unsigned int  HDP=799;
unsigned int  HT=850;
unsigned int  HPS=51;
unsigned int  LPS=3;
unsigned char HPW=48;

unsigned int  VDP=479;
unsigned int  VT=530;
unsigned int  VPS=50;
unsigned int  FPS=30;
unsigned char   VPW=3;
 */


unsigned int  HDP=479;

unsigned int  HT=531;
unsigned int  HPS=43;
unsigned int  LPS=8;
unsigned char HPW=1;
unsigned int  VDP=271;
unsigned int  VT=288;
unsigned int  VPS=12;
unsigned int  FPS=4;
unsigned char VPW=10;

extern void lcd_rst(void);
void Delay(__IO uint32_t nCount);

void SetScrollArea(unsigned int top, unsigned int scroll, unsigned int bottom);
void SetScrollStart(unsigned int line);
void SetTearingCfg(unsigned char state, unsigned char mode);

/****************************************************************************
* ��    �ƣ�void Delay(__IO uint32_t nCount)
* ��    �ܣ���ʱ����
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}
/****************************************************************************
* ��    �ƣ�LCD_WR_REG(unsigned int index)
* ��    �ܣ�FSMCд��ʾ���Ĵ�����ַ����
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void LCD_WR_REG(unsigned int index)
{
	*(__IO uint16_t *) (Bank1_LCD_C)= index;

}

/****************************************************************************
* ��    �ƣ�void LCD_WR_CMD(unsigned int index,unsigned int val)
* ��    �ܣ�FSMCд��ʾ���Ĵ������ݺ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void LCD_WR_CMD(unsigned int index,unsigned int val)
{	
	*(__IO uint16_t *) (Bank1_LCD_C)= index;	
	*(__IO uint16_t *) (Bank1_LCD_D)= val;
}

/****************************************************************************
* ��    �ƣ�unsigned short LCD_RD_data(void)
* ��    �ܣ�FSMC����ʾ��16λ���ݺ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
unsigned short LCD_RD_data(void){
	unsigned int a=0;
	a=*(__IO uint16_t *) (Bank1_LCD_D);   //������ʵ��16λ��������	  
	return(a);	
}

/****************************************************************************
* ��    �ƣ�LCD_WR_Data(unsigned int val)
* ��    �ܣ�FSMCд16λ���ݺ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void    LCD_WR_Data(unsigned int val)
{   
	*(__IO uint16_t *) (Bank1_LCD_D)= val; 	
}


/****************************************************************************
* ��    �ƣ�void LCD_Init1(void)
* ��    �ܣ�4.3��TFT ��ʼ������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/ 
void LCD_Init1(void)
{
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
    Delay(0xAFFFF);					   
    GPIO_SetBits(GPIOE, GPIO_Pin_1 );		 	 
	Delay(0xAFFFF);

    LCD_WR_REG(0x00E2);	        //PLL ��Ƶ��120M   PLL=10*36/3=120M
	LCD_WR_Data(0x0023);	    //N=0x36 for 6.5M, 0x23 for 10M crystal
	LCD_WR_Data(0x0002);
	LCD_WR_Data(0x0004);
	
	LCD_WR_REG(0x00E0);         
	LCD_WR_Data(0x0001);
	Delay(0xAFFF);
	LCD_WR_REG(0x00E0);
	LCD_WR_Data(0x0003);		// PLL ʹ��  PLL��Ϊʱ��
	Delay(0xAFFF);
	LCD_WR_REG(0x0001);         // �����λ
	Delay(0xAFFF);
	LCD_WR_REG(0x00E6);	        //PLL setting for PCLK, depends on resolution
	LCD_WR_Data(0x0001);
	LCD_WR_Data(0x0033);
	LCD_WR_Data(0x0032);

	LCD_WR_REG(0x00B0);	        //����LCDģʽ
	LCD_WR_Data(0x0000);
	LCD_WR_Data(0x0000);
	LCD_WR_Data((HDP>>8)&0X00FF);  //Set HDP
	LCD_WR_Data(HDP&0X00FF);
    LCD_WR_Data((VDP>>8)&0X00FF);  //Set VDP
	LCD_WR_Data(VDP&0X00FF);
    LCD_WR_Data(0x0000);

	LCD_WR_REG(0x00B4);	//HSYNC
	LCD_WR_Data((HT>>8)&0X00FF);  //Set HT
	LCD_WR_Data(HT&0X00FF);
	LCD_WR_Data((HPS>>8)&0X00FF);  //Set HPS
	LCD_WR_Data(HPS&0X00FF);
	LCD_WR_Data(HPW);			   //Set HPW
	LCD_WR_Data((LPS>>8)&0X00FF);  //Set HPS
	LCD_WR_Data(LPS&0X00FF);
	LCD_WR_Data(0x0000);

	LCD_WR_REG(0x00B6);	//VSYNC
	LCD_WR_Data((VT>>8)&0X00FF);   //Set VT
	LCD_WR_Data(VT&0X00FF);
	LCD_WR_Data((VPS>>8)&0X00FF);  //Set VPS
	LCD_WR_Data(VPS&0X00FF);
	LCD_WR_Data(VPW);			   //Set VPW
	LCD_WR_Data((FPS>>8)&0X00FF);  //Set FPS
	LCD_WR_Data(FPS&0X00FF);

	LCD_WR_REG(0x00BA);
	LCD_WR_Data(0x000F);    //GPIO[3:0] out 1

	LCD_WR_REG(0x00B8);
	LCD_WR_Data(0x0007);    //GPIO3=input, GPIO[2:0]=output
	LCD_WR_Data(0x0001);    //GPIO0 normal

	LCD_WR_REG(0x0036);            //rotation
	LCD_WR_Data(0x0000);


	LCD_WR_REG(0x00F0);             //16λ  565ģʽ
	LCD_WR_Data(0x0003);


	Delay(0xAFFF);

	LCD_clear();
	LCD_WR_REG(0x0029); //display on

	LCD_WR_REG(0x00BE); //set PWM for B/L
	LCD_WR_Data(0x0006);
	LCD_WR_Data(0x0080);
	
	LCD_WR_Data(0x0001);
	LCD_WR_Data(0x00f0);
	LCD_WR_Data(0x0000);
	LCD_WR_Data(0x0000);

	LCD_WR_REG(0x00d0);//���ö�̬����������� 
	LCD_WR_Data(0x000d);

   			

}
/****************************************************************************
* ��    �ƣ�void LCD_clear(void)
* ��    �ܣ�����
* ��ڲ�����
* ���ڲ�����
* ˵    ����
* ���÷�����
****************************************************************************/
void LCD_clear(void)
{
    unsigned int l=480,w;

	LCD_WR_REG(0x002A);	
	LCD_WR_Data(0);	    
	LCD_WR_Data(0);
	LCD_WR_Data(HDP>>8);	    
	LCD_WR_Data(HDP&0x00ff);
    LCD_WR_REG(0x002b);	
	LCD_WR_Data(0);	    
	LCD_WR_Data(0);
	LCD_WR_Data(VDP>>8);	    
	LCD_WR_Data(VDP&0x00ff);
	LCD_WR_REG(0x002c);
	while(l--)
	{
	    for(w=0;w<272;w++)
		{    
          	LCD_WR_Data(0xf800);
		}
	}
}


/******************* (C) COPYRIGHT 2011 �ܶ�STM32 *****END OF FILE****/
