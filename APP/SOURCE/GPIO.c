#include "Includes.h"

//#include <stm32f10x_hd.h>
/*******************************************************************************
* Function Name  : 
* Description    : Configure GPIO Pin
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/


void delay_us(u32 delay_count)//accurate delay 1uS
{
   u32 temp = 0;
   temp = delay_count;
   SysTick->CTRL &= (u32)0XFFFFFFFB;
   temp = 9 * temp;

   SysTick->LOAD = temp;      
   SysTick->VAL=0x00;   
   SysTick->CTRL=0x01;     
   
   temp = 0;
   do 
   { 
     temp=SysTick->CTRL; 
   } 
   while(temp&0x01&&!(temp&(1<<16)));   
   
   SysTick->CTRL=0x00;       
   SysTick->VAL =0X00;
}

/****************************************************************************
* Name: void Delay_Common(unsigned int DelayCnt)
* Function: Delay function
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/  
void Delay_Common(unsigned int DelayCnt)
{
 unsigned int i;
 for(i=0;i<DelayCnt;i++);
}


