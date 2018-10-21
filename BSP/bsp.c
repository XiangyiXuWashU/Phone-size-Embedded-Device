/****************************************************************************
* 文件名: bsp.c
* 内容简述:硬件平台的初始化		
*/
#include "..\App\INC\includes.h"

extern OS_EVENT* ADC_SEM;
extern volatile unsigned  short int  ADC_TIMEOUT;
extern volatile unsigned char rec_f,tx_flag,ADC_R;
extern volatile unsigned long Rec_Len;

extern double Temperature;
extern double BiasCurrent;	
extern double ModulateCurrent;

extern int FlashTemperature;    //FlashTemperature=Temperature*100
extern int FlashBiasCurrent;    //FlashBiasCurrent=BiasCurrent*100
extern int FlashModulateCurrent;//FlashModulateCurrent=FlashModulateCurrent*100

extern u8  FlashData[8];
extern int FlashDataIndex;


extern volatile unsigned  short int  CalibratedValue[1000]; //Calibrated data
 
#define ADC1_DR_Address    ((u32)0x4001244C)  	            //ADC1 peripheral address
/* Defines the SPI chip select control for the touch chip */
#define TP_CS()  GPIO_ResetBits(GPIOB,GPIO_Pin_12)	  
#define TP_DCS() GPIO_SetBits(GPIOB,GPIO_Pin_12)

void tp_Config(void) ;
u16 TPReadX(void);
u16 TPReadY(void);
extern void FSMC_LCD_Init(void); 


/****************************************************************************
* Name: void RCC_Configuration(void)
* Function: System clock configuration is 72MHZ, peripheral clock configuration
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/ 
void RCC_Configuration(void){
  SystemInit();	  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
                RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD| RCC_APB2Periph_GPIOE, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* DMA2 clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  /* TIM2 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

/****************************************************************************
* Name: void GPIO_Configuration(void)
* Function: General IO port configuration
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/  
void GPIO_Configuration(void)
{

}

/****************************************************************************
* Name: void NVIC_Configuration(void)
* Function: Interrupt source configuration function
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/ 
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  //Enable DMA1_Channel1 Interuption
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; //USART1_IRQChannel
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);	
	
}


/****************************************************************************
* Name: void BSP_Init(void)
* Function: Struggle board initialization function
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/  
void BSP_Init(void)
{
	int i=0;
  RCC_Configuration();  	//System clock initialization and port peripheral clock enable
  NVIC_Configuration();		//Interrupt source configuration
  tp_Config();			//SPI2 touch circuit init
  FSMC_LCD_Init();		//FSMC TFT interface init
  Synchronize_ADC_DAC();        //ADC and DAC sync
  InitData();                   //Inite data

	
  WM_SetCreateFlags(WM_CF_MEMDEV); 
  SwitchTEC(0);                 //Turn off TEC
  SwitchLaser(0);               //Turn off Laser
	
  ReadFlashData(0x0807FFF8,6);
	
  Temperature = (float)(FlashData[0]+(FlashData[1]<<8))/100;
  BiasCurrent = (float)(FlashData[2]+(FlashData[3]<<8))/100;
  ModulateCurrent = (float)(FlashData[4]+(FlashData[5]<<8))/100;

  SetTemperature(Temperature);  //Set Temperature
  SetCurrent(BiasCurrent,ModulateCurrent); //Set Current
	
  IWDG_ReloadCounter();	
    for(i=0;i<1000;i++)
    {
        CalibratedValue[i]=1;
    }

  InitUSB();                 //USB init
  delay_us(500000);
	
  InitIWDG();	             //Watchdog init
}

/****************************************************************************
* Name: void OS_CPU_SysTickInit(void)
* Function: ucos system tick clock initialization Initial setting is 10ms once beat
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
void  OS_CPU_SysTickInit(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    INT32U         cnts;
    RCC_GetClocksFreq(&rcc_clocks);		                //Get the value of the system clock
    cnts = (INT32U)rcc_clocks.HCLK_Frequency/OS_TICKS_PER_SEC;	//Calculate the value of the clock beat
    SysTick_Config(cnts);					//Set the clock beat
}

/****************************************************************************
* Name: void tp_Config(void)
* Function: TFT touch screen control initialization
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
void tp_Config(void) 
{ 
  GPIO_InitTypeDef  GPIO_InitStructure; 
  SPI_InitTypeDef   SPI_InitStructure; 

  /* SPI2 timer enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);	
 
  /* SPI2 SCK(PB13)、MISO(PB14)、MOSI(PB15) Configuration*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//50MHZ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	        //Reuse mode
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Chip Select Control Settings for SPI2 Touch Chip PB12 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//50MHZ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//Push-pull output mode
  GPIO_Init(GPIOB, &GPIO_InitStructure);
   
  
  /* SPI2 bus configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   //Full duplex
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;			       //Main mode
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		       //8 bits
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//Clock polarity SCK remains low during idle state
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		//Clock phase data sampling starts from the first clock edge
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//Software generates NSS
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;  //Baud rate control SYSCLK/64
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;		//Data high in front
  SPI_InitStructure.SPI_CRCPolynomial = 7;			//The initial value of the CRC polynomial register is 7
  SPI_Init(SPI2, &SPI_InitStructure);
  
  /* SPI2 Enable */
  SPI_Cmd(SPI2,ENABLE);  
}

/****************************************************************************
* Name: unsigned char SPI_WriteByte(unsigned char data)
* Function: SPI2 write function
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/  
unsigned char SPI_WriteByte(unsigned char data) 
{ 
  unsigned char Data = 0;
  //Waiting for the send buffer empty
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET); 
  //Send one byte
  SPI_I2S_SendData(SPI2,data); 

  //Waiting for a byte to be received
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET); 
  //Get the byte
  Data = SPI_I2S_ReceiveData(SPI2); 

  //Return received bytes
  return Data; 
}  

/****************************************************************************
* Name: void SpiDelay(unsigned int DelayCnt)
* Function: SPI2 write delay function
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/  
void SpiDelay(unsigned int DelayCnt)
{
 unsigned int i;
 for(i=0;i<DelayCnt;i++);
}

/****************************************************************************
* Name: u16 TPReadX(void)
* Function: Touch screen X-axis data readout
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/  
u16 TPReadX(void)
{ 
   u16 x=0;
   TP_CS();	                //Select XPT2046
   SpiDelay(10);		//delay
   SPI_WriteByte(0xD0);	        //Set the X axis read flag
   SpiDelay(10);		//delay
   x=SPI_WriteByte(0x00);	//Continuously read 16-bit data
   x<<=8;
   x+=SPI_WriteByte(0x00);
   SpiDelay(10);		//disable XPT2046
   TP_DCS(); 					    								  
   x = x>>3;			//Shift converted to 12-bit valid data 0-4095
   return (x);	
}
/****************************************************************************
* Name: u16 TPReadY(void)
* Function: touch screen Y-axis data readout
* Entry parameters: none
* Export parameters: none
* Description:
* Call method:
****************************************************************************/
u16 TPReadY(void)
{
   u16 y=0;
   TP_CS();	                //Select XPT2046
   SpiDelay(10);		//delay
   SPI_WriteByte(0x90);		//Set the Y axis read flag
   SpiDelay(10);		//delay
   y=SPI_WriteByte(0x00);	//Continuously read 16-bit data
   y<<=8;
   y+=SPI_WriteByte(0x00);
   SpiDelay(10);		//disable XPT2046
   TP_DCS(); 					    								  
   y = y>>3;			//Shift converted to 12-bit valid data 0-4095
   return (y);
}


