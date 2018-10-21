#include "Includes.h"

volatile unsigned  short int  ADC_ConvertedValue[6000],ADC_ResultValue[6],PD_ADCValue[1000];//500 data per channel
char ADC_STR1[8];	  //ADC Int to Char PA0/VS
char ADC_STR2[8];	  //ADC Int to Char PA1/VMCU
char ADC_STR3[8];	  //ADC Int to Char PC0/REAL I
char ADC_STR4[8];	  //ADC Int to Char PC2/REAL T
char ADC_STR5[8];	  //ADC Int to Char PC3/GRAPH Y
char ADC_STR6[8];	  //ADC Int to Char PC4/VLD

char ADC_STR7[8];	  //PeakX

char SET_STRT[8];	  //SET T String
char SET_STRI[8];	  //SET I String
char SCANAMPL[8];         //Real Scan Amplitude
/*******************************************************************************
* Function Name  : ADC_Configuration
* Description    : Configure the ADC.
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
void ADC_Configuration(void)
{ 
   ADC_InitTypeDef ADC_InitStructure;
   GPIO_InitTypeDef GPIO_InitStructure;
   DMA_InitTypeDef DMA_InitStructure;
	
   /* Enable DMA clock */
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
   RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //ADC sampling 6 frequency division 72M/6=12M

   /* Enable ADC1,GPIOA, GPIOC clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO , ENABLE);

   /* DMA channel1 configuration ----------------------------------------------*/
    //Enable DMA
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;      //DMA Channel 1 address
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue; //DMA Transfer address
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;		//Transfer direction
    DMA_InitStructure.DMA_BufferSize = 6000;			//Transfer memory size, 60 16 bits
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;		//Transfer memory address increment
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//ADC1 16 Bits
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//The destination address of the transfer is 16-bit width
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    /* Allow DMA1 channel 1 transmission end interrupt */
    DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);

    //Enable DAM1
    DMA_Cmd(DMA1_Channel1, ENABLE);

    //Set the AD analog input port as an input rule channel
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* TIM2 Configuration */
    //ADC Configuration
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;    //ADC1 is Working in standalone mode
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;	      //performed in Scan (multichannels)or Single (one channel) mode.
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;   //the conversion is performed in Continuous or Single mode
    ADC_InitStructure.ADC_ExternalTrigConv =ADC_ExternalTrigConv_T2_CC2;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC data right align
    ADC_InitStructure.ADC_NbrOfChannel = 6;               //Specifies the number of ADC channels that are sequentially subjected to rule conversion. This number ranges from 1 to 16
    ADC_Init(ADC1, &ADC_InitStructure);

    /* ADC1 regular channels configuration */

    //ADC  TCONV = 7.5(ADC_SampleTime) + 12.5(ADC_CLK)=20 Time Cycle  20*1/12M=1.67uS
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 5, ADC_SampleTime_71Cycles5);	  //PC3/GRAPH Y  ADC_SampleTime_55Cycles5
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_7Cycles5);	  //PC0/REAL I
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 4, ADC_SampleTime_7Cycles5);	  //PC2/REAL T
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 6, ADC_SampleTime_7Cycles5);	  //PC4/VLD
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5);	  //PA0/VS
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_7Cycles5);	  //PA1/VMCU

    //Enabel ADC1 DMA
    ADC_DMACmd(ADC1, ENABLE);

    //Enabel ADC1
    ADC_Cmd(ADC1, ENABLE);

    //Initialize the ADC1 calibration register
    ADC_ResetCalibration(ADC1);

    //Check if ADC1 calibration register initialization is complete
    while(ADC_GetResetCalibrationStatus(ADC1));

    //Start calibrating ADC1
    ADC_StartCalibration(ADC1);

    //Check if calibration is complete
    while(ADC_GetCalibrationStatus(ADC1));

    //ADC1 conversion start
    ADC_ExternalTrigConvCmd(ADC1,ENABLE);
}

void Synchronize_ADC_DAC(void)
{
    TIM2_Configuration();
    ADC_Configuration();
    InitDAC();

    TIM_Cmd(TIM2, ENABLE);
}


void ReadVoltage()
{
  int AD_value=0;
  int i,j;
				 
  if(DMA_GetITStatus(DMA1_IT_TC1))	 //Conversion completed
    {
    /* Oversampling the ADC to ensure accuracy*/
     for(i=0;i<6;i++)
        {
            AD_value=0;
            for(j=0;j<1000;j++)
                {
                 AD_value += ADC_ConvertedValue[j*6+i];
                }
            AD_value =  AD_value/1000;
            AD_value =  (AD_value*3300)/4096;
            ADC_ResultValue[i] = AD_value;
        }

     for(j=0;j<1000;j++)
        {
            PD_ADCValue[j]=(ADC_ConvertedValue[j*6+4])*0.806;
        }

     DMA_ClearITPendingBit(DMA1_IT_GL1);     //Clear interrupt flag
    }
}




