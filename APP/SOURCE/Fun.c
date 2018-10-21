#include <stddef.h>           /* needed for definition of NULL */
#include "..\GUIinc\GUI.h"
#include "..\GUIinc\WM.h"	 
#include "..\GUIinc\DIALOG.h" 
#include "..\GUIinc\FRAMEWIN.h"	
#include "..\GUI_GRAPH\GRAPH.h" 
#include "..\GUI_GRAPH\GRAPH_Private.h" 
#include "..\App\INC\includes.h"
#include <stdlib.h>

extern OS_EVENT* ADC_SEM;
extern volatile unsigned  short int  ADC_TIMEOUT;
extern volatile unsigned char ADC_R;
 
extern char ADC_STR1[8];	  //ADC PA0/VS
extern char ADC_STR2[8];	  //ADC PA1/VMCU
extern char ADC_STR3[8];	  //ADC PC0/REAL I
extern char ADC_STR4[8];	  //ADC PC2/REAL T
extern char ADC_STR5[8];	  //ADC PC3/GRAPH Y
extern char ADC_STR6[8];	  //ADC PC4/VLD
extern char ADC_STR7[8];	  //Peak X

extern char SET_STRT[8];	  //SET T String
extern char SET_STRI[8];	  //SET I String
extern char SCANAMPL[8];	  //Real Scan Amplitude

extern double Temperature;
extern double BiasCurrent;	
extern double ModulateCurrent;

extern int FlashTemperature;    //FlashTemperature=Temperature*100
extern int FlashBiasCurrent;    //FlashBiasCurrent=BiasCurrent*100
extern int FlashModulateCurrent;//FlashModulateCurrent=FlashModulateCurrent*100

extern volatile unsigned  short int  PD_ADCValue[1000];
extern volatile unsigned  short int  ADC_ResultValue[6];

static void _OnValueChanged(WM_HWIN hDlg, int Id);
static void ButtonClick(WM_HWIN hDlg, int Id);
void SwitchTEC(int State);

void Fun(void);

extern u8  FlashData[8];
extern int FlashDataIndex;	

volatile unsigned  short int FinalY[1000];

volatile unsigned  short int  CalibratedValue[1000];

static float SCALING=  6  ; //Y Axis is 180 pixel, 160*SCALING<= Max PD Volate
static I16 MAX =    960 ;   //The max volte of PD(mV)
static I16 MIN =    300 ;   //The min volte of PD(mV)
static int AUTOY=     1;    //Update the Y Axis Value

/* ucgui type define*/
  WM_HWIN hWin1, hItem;;
  WM_HWIN text8,text9,text10,text11,text12,text13,text14,text15,text16,text17,text18,text19;	
	WM_HWIN Button0,Button1,Button2,Button3,Button4,Button5,Button6,Button7,Button8,Button9,Button10,Button11;
  GUI_COLOR DesktopColorOld;
  const GUI_FONT* pFont = &GUI_FontComic24B_1;
	GRAPH_DATA_Handle  hDataYT;  


	I16 PeakX;   //Find Peak X Position

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GRAPH_SCALE_Handle _hScaleV;   /* Handle of vertical scale */
static GRAPH_SCALE_Handle _hScaleH;   /* Handle of horizontal scale */

static  I16 _aValue[320];

const I16 DisplayData[320] =
                              {
																	159,158,157,157,157,156,156,155,155,154,153,153,
																	152,152,151,152,150,151,150,150,149,148,148,148,
																	147,146,147,146,146,145,145,145,144,144,144,143,
																	143,143,143,143,142,142,142,142,141,141,140,141,
																	140,140,140,139,139,138,139,139,138,138,137,138,
																	138,138,137,137,138,136,137,136,136,136,137,136,
																	136,136,135,135,136,135,135,135,135,134,134,133,
																	134,134,133,133,132,133,132,132,130,130,129,130,
																	130,129,128,128,128,128,127,127,127,125,126,125,
																	124,124,122,122,121,120,120,119,119,118,117,115,
																	114,113,113,112,110,110,109,107,104,103,101,99,97,
																	94,93,89,87,84,82,80,78,74,72,68,65,61,58,54,50,45,
																	41,37,33,30,26,22,21,18,15,14,12,9,7,7,6,5,6,6,5,5,
																	6,7,8,9,10,11,12,15,17,19,21,23,25,27,31,32,35,37,
																	40,42,44,47,49,51,53,55,57,59,62,64,65,68,69,71,74,
																	75,77,79,80,83,84,86,87,88,90,92,93,95,96,99,99,100,
																	101,103,104,106,107,108,109,111,111,112,113,115,115,
																	117,117,119,119,120,121,122,123,123,124,125,126,126,
																	127,127,129,129,129,130,131,132,132,134,133,134,135,
																	135,136,137,137,138,138,139,139,140,140,141,140,142,
																	142,142,143,143,144,145,145,145,146,147,146,146,147,
																	148,149,148,149,149,151,150,150,151,151,151,151,152,
																	152,152,153,154,153,154,153,154,154,154,155,155,155,
																	156,156,156,156,157,157,156,
                               
                              };

/* �����˶Ի�����Դ�б� */
static const GUI_WIDGET_CREATE_INFO aDialogCreate1[] = 
{
  //��������1�� ��С��320X120  ԭ����0��0
  { FRAMEWIN_CreateIndirect, "Portable WGM Lab", 0,  0,   0, 480, 272, FRAMEWIN_CF_ACTIVE },
	
	//����TEXT�ؼ�������Ǵ����0��0����С40X12  
  { TEXT_CreateIndirect,     "SET T:",  GUI_ID_TEXT0,     0,   0,  40,  12, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "26.00",    GUI_ID_TEXT8,     40,   0,  40,  12, TEXT_CF_LEFT },	

  { TEXT_CreateIndirect,     "REAL T:",  GUI_ID_TEXT1,   80,   0,  40,  12, TEXT_CF_LEFT},	
  { TEXT_CreateIndirect,     "0.00",    GUI_ID_TEXT9,    120,   0,  40,  12, TEXT_CF_LEFT },	
	
  { TEXT_CreateIndirect,     "SET I:",  GUI_ID_TEXT2,   0,  20,  40,  12, TEXT_CF_LEFT },
  { TEXT_CreateIndirect,     "145",    GUI_ID_TEXT10,    40,   20,  40,  12, TEXT_CF_LEFT },	
 
  { TEXT_CreateIndirect,     "REAL I:", GUI_ID_TEXT3,  80, 20,  40, 12, TEXT_CF_LEFT},
  { TEXT_CreateIndirect,     "0.0",    GUI_ID_TEXT11,    120,   20,  40,  12, TEXT_CF_LEFT },	
	
	 
  { TEXT_CreateIndirect,     "VS:",  GUI_ID_TEXT4,     160,   0,  40,  12, TEXT_CF_LEFT },
	{ TEXT_CreateIndirect,     "0.00",    GUI_ID_TEXT12,    200,   0,  40,  12, TEXT_CF_LEFT },	

  { TEXT_CreateIndirect,     "VMCU:",  GUI_ID_TEXT5,   240,   0,  40,  12, TEXT_CF_LEFT},	
	{ TEXT_CreateIndirect,     "0.00",    GUI_ID_TEXT13,   280,   0,  40,  12, TEXT_CF_LEFT },	

  { TEXT_CreateIndirect,     "VLD:",  GUI_ID_TEXT6,   160,  20,  40,  12, TEXT_CF_LEFT },
	{ TEXT_CreateIndirect,     "0.00",    GUI_ID_TEXT14,    200,  20,  40,  12, TEXT_CF_LEFT },	
  
  { TEXT_CreateIndirect,     "SCAN:", GUI_ID_TEXT7,  240, 20,  40, 12, TEXT_CF_LEFT}, 
  { TEXT_CreateIndirect,     "40",    GUI_ID_TEXT15,    280,   20,  40,  12, TEXT_CF_LEFT },		
	
	{ TEXT_CreateIndirect,     "Peak to Peak:", GUI_ID_TEXT16,  0, 200,  160, 20, TEXT_CF_LEFT}, 
  { TEXT_CreateIndirect,     "0.000",    GUI_ID_TEXT17,    200,   200,  120,  20, TEXT_CF_LEFT },	
	{ TEXT_CreateIndirect,     "Peak X:", GUI_ID_TEXT18,  0, 220,  160, 20, TEXT_CF_LEFT}, 
  { TEXT_CreateIndirect,     "0.000",    GUI_ID_TEXT19,    200,   220,  120,  20, TEXT_CF_LEFT },	
	
	
	//������ť
  { BUTTON_CreateIndirect,   "LASER OFF",     GUI_ID_BUTTON0,       320,    0,  80, 40 },
  { BUTTON_CreateIndirect,   "TEC OFF",    GUI_ID_BUTTON1,      400,   0,  80, 40 }, 
	{ BUTTON_CreateIndirect,   "Scan+",   GUI_ID_BUTTON2,       320,    40,  80, 40 },
  { BUTTON_CreateIndirect,   "Scan-",   GUI_ID_BUTTON3,       400,   40,  80,  40},
	{ BUTTON_CreateIndirect,   "Temp+",     GUI_ID_BUTTON4,       320,    80,  80, 40 },
  { BUTTON_CreateIndirect,   "Temp-",    GUI_ID_BUTTON5,      400,   80,  80, 40 }, 
	{ BUTTON_CreateIndirect,   "Current+",   GUI_ID_BUTTON6,       320,    120,  80, 40 },
  { BUTTON_CreateIndirect,   "Current-",   GUI_ID_BUTTON7,       400,   120,  80,  40},
	{ BUTTON_CreateIndirect,   "AUTOY",   GUI_ID_BUTTON8,       320,   160,  80,  40},
	{ BUTTON_CreateIndirect,   "Calibrate",   GUI_ID_BUTTON9,       400,   160,  80,  40},
	{ BUTTON_CreateIndirect,   "Clear Cal",   GUI_ID_BUTTON10,       320,   200,  80,  40},	
	{ BUTTON_CreateIndirect,   "Backup2",   GUI_ID_BUTTON11,       400,   200,  80,  40},	

	
	{ GRAPH_CreateIndirect,     0,         GUI_ID_GRAPH0    ,  0,   40, 320, 160 },  

};



/*********************************************************************

*/
I16 FindArrayMax(volatile unsigned short int *Array,int ArrayNumber)
{
	I16 Max=Array[0];
	int i=0;
	for(i=0;i<ArrayNumber;i++)
		{
			if(Array[i]>=Max)
			{
				Max = Array[i];		
			} 
	  }
  
  return Max;
}

I16 FindArrayMin(volatile unsigned short int *Array,int ArrayNumber)
{
	I16 Min=Array[0];
	int i=0;
	for(i=0;i<ArrayNumber;i++)
		{
			if(Array[i]<=Min)
			{
				Min = Array[i];		
			} 
	  }
  
  return Min;
}

I16 FindPeakX(I16 *Array,int ArrayNumber)
{
	I16 Min=Array[0];
	int i=0,PeakXPosition=0;
	for(i=0;i<ArrayNumber;i++)
		{
			if(Array[i]<=Min)
			{
				Min = Array[i];	
        PeakXPosition=i;				
			} 
	  }
  
  return PeakXPosition;
}

void AutoAdjustY(volatile unsigned  short int *FinalY)
{
	
	MAX=FindArrayMax(FinalY,1000);
	MIN=FindArrayMin(FinalY,1000);
	SCALING=(MAX/160)*1.2;
	
}

void UpdatePDValues(void) 
{
  int i,j;

  memset(_aValue,0,640);
	
	for(i=0;i<1000;i++)
	 {
		 FinalY[i]=PD_ADCValue[i]/CalibratedValue[i];	 
	 }
	 
	IWDG_ReloadCounter();	
	 
	if(AUTOY)
	{
  	 AutoAdjustY(FinalY);
	}
	
  for (i = 0; i < 320; i++)
	  {		
			for(j=0;j<3;j++)
			{
		   _aValue[i] = _aValue[i]+FinalY[i*3+j];			
			}
			
			_aValue[i] =  ((_aValue[i]/(3*SCALING))-(MIN/SCALING))*160/((MAX/SCALING)-(MIN/SCALING)); 
			
			_aValue[i] =	DisplayData[i];
    }
	  
}


void FlashProcess(void)   //Store Laser Parameter into flash
{
		FlashTemperature=(int)(Temperature*100);
  	FlashBiasCurrent=(int)(BiasCurrent*100);
  	FlashModulateCurrent=(int)(ModulateCurrent*100);
		EraseAllFlash();
		Delay_Common(10000);
		FlashDataIndex=6;
		FlashData[0]=FlashTemperature&0xFF;
		FlashData[1]=(FlashTemperature>>8)&0xFF;
	  FlashData[2]=FlashBiasCurrent&0xFF;
	  FlashData[3]=(FlashBiasCurrent>>8)&0xFF;
	  FlashData[4]=FlashModulateCurrent&0xFF;
	  FlashData[5]=(FlashModulateCurrent>>8)&0xFF;	
	
		StoreIntoFlash(); 	
		Delay_Common(10000);
}

/*********************************************************************
*
*       _UserDraw
*
* Purpose:
*   This routine is called by the GRAPH object before anything is drawn
*   and after the last drawing operation.
*/
static void _UserDraw(WM_HWIN hWin, int Stage) {
  if (Stage == GRAPH_DRAW_LAST) {
    char acText[] = "";
    GUI_RECT Rect, RectInvalid;
    int FontSizeY;
    GUI_SetFont(&GUI_Font13_ASCII);
    FontSizeY = GUI_GetFontSizeY();
    WM_GetInsideRect(&Rect);
    WM_GetInvalidRect(hWin, &RectInvalid);
    Rect.x1 = Rect.x0 + FontSizeY;
    GUI_SetColor(GUI_RED);
    GUI_DispStringInRectEx(acText, &Rect, GUI_TA_HCENTER, strlen(acText), GUI_ROTATE_CCW);
  }
}


/****************************************************************************
* ��    �ƣ�static void _cbCallback(WM_MESSAGE * pMsg)  
* ��    �ܣ�����ص����� 
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
static void _cbCallback(WM_MESSAGE * pMsg) {    
  int NCode, Id;
  WM_HWIN hDlg;	 

	
	hDlg = pMsg->hWin;	  
  switch (pMsg->MsgId) {     
    case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);      /*��ô��岿����ID*/
      NCode = pMsg->Data.v;                 /*�������� */
      switch (NCode) {	          
        case WM_NOTIFICATION_VALUE_CHANGED: /*���岿����ֵ���ı� */	   
          _OnValueChanged(hDlg, Id);		 
          break;
				
				case WM_NOTIFICATION_CLICKED:					
          ButtonClick(hDlg, Id);
				  break;
				
        default:
          break;
      }
      break;  
	 case WM_INIT_DIALOG:
	  hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
    /* Add graphs */
    GRAPH_SetColor(hItem,GUI_WHITE,GRAPH_CI_BK);
    /* Set graph attributes */
    GRAPH_SetGridDistY(hItem, 20);
	  GRAPH_SetGridDistX(hItem, 40);	 
    GRAPH_SetGridVis(hItem, 1);

    GRAPH_SetUserDraw(hItem, _UserDraw);
    /* Create and add vertical scale */
    _hScaleV = GRAPH_SCALE_Create( 20, GUI_TA_RIGHT, GRAPH_SCALE_CF_VERTICAL, 20);
		GRAPH_SCALE_SetFactor(_hScaleV, 1);
    GRAPH_SCALE_SetNumDecs(_hScaleV, 0);	   
    GRAPH_SCALE_SetTextColor(_hScaleV, GUI_DARKGREEN);
    GRAPH_AttachScale(hItem, _hScaleV);
    /* Create and add horizontal scale */
	 
    _hScaleH = GRAPH_SCALE_Create(152, GUI_TA_HCENTER, GRAPH_SCALE_CF_HORIZONTAL, 40);
	  GRAPH_SCALE_SetFactor(_hScaleH, 1);
    GRAPH_SCALE_SetTextColor(_hScaleH, GUI_DARKGREEN);
    GRAPH_AttachScale(hItem, _hScaleH);
	 	
    default:
      WM_DefaultProc(pMsg);
  }
}

/****************************************************************************
* ��    �ƣ�static void _OnValueChanged(WM_HWIN hDlg, int Id) 
* ��    �ܣ�ֵ���ı�Ķ���
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/	   
static void _OnValueChanged(WM_HWIN hDlg, int Id)
{

	
  if (Id == GUI_ID_SLIDER0) //
		{	
      					

					 
    }	
  else if (Id == GUI_ID_SLIDER1)//slider1 ��ֵ���ı�
		{					 
			
    }	
  else 
	  {
	  }
 
}

static void ButtonClick(WM_HWIN hDlg, int Id)
{
   u8 ReadValue;
 
	 BUTTON_Handle hButton;
	
	 int i=0;
	
	 switch (Id) 
		{
		  case GUI_ID_BUTTON0: //Turn ON/OFF Laser
				
					 hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);		
	         ReadValue = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_7);
					 if(ReadValue==1)
							 {
								 SwitchLaser(1);								 
								 BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_RED);
								 BUTTON_SetText(hButton,"LASER ON");
							 }
						else
							 {
								 SwitchLaser(0);								 
								 BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
								 BUTTON_SetText(hButton,"LASER OFF");
							 }
				  
				break;
					 
			case GUI_ID_BUTTON1:   //Turn ON/OFF TEC

			     hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON1);		
	         ReadValue = GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_6);
					 if(ReadValue==0)
							 {
								 SwitchTEC(0);								 
								 BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
								 BUTTON_SetText(hButton,"TEC OFF");
							 }
						else
							 {
								 SwitchTEC(1);								
								 BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_RED);
								 BUTTON_SetText(hButton,"TEC ON");
							 }
				  
			  break;
			case GUI_ID_BUTTON2:  //SCAN+
				
				text15 = WM_GetDialogItem(hWin1, GUI_ID_TEXT15);
				ModulateCurrent=ModulateCurrent+2;		
			  if(ModulateCurrent>=40)
				{
				  ModulateCurrent=40;
				}
				else if(ModulateCurrent<=0)
				{
					ModulateCurrent=0;
				}
				else
				{

			  }	
				
  			SetCurrent(BiasCurrent, ModulateCurrent);
				
				FlashProcess();
				
				F2S(ModulateCurrent, SCANAMPL,0);
				TEXT_SetText(text15,SCANAMPL);
				
				break;
				
		  case GUI_ID_BUTTON3:  //SCAN-
				
			  text15 = WM_GetDialogItem(hWin1, GUI_ID_TEXT15);
				ModulateCurrent=ModulateCurrent-2;			
			  if(ModulateCurrent>=40)
				{
				  ModulateCurrent=40;
				}
				else if(ModulateCurrent<=0)
				{
					ModulateCurrent=0;
				}
				else
				{

			  }	
				
  			SetCurrent(BiasCurrent, ModulateCurrent);
				
				FlashProcess();
				
				F2S(ModulateCurrent, SCANAMPL,0);
				TEXT_SetText(text15,SCANAMPL);
				
				break;
					 
			case GUI_ID_BUTTON4:  //T+
				
				text8 = WM_GetDialogItem(hWin1, GUI_ID_TEXT8);
				Temperature=Temperature+0.05;			
				if(Temperature>=50)
				{
				  Temperature=50;
				}
				else if(Temperature<=0)
				{
					Temperature=0;
				}
				else
				{

			  }
			  SetTemperature(Temperature);
				
				FlashProcess();

				
        if(Temperature>=25)
				{
			    F2S(Temperature, SET_STRT,2);
				}
        else
				{
			    F2S(Temperature+0.000000001, SET_STRT,2);
				}
			  TEXT_SetText(text8,SET_STRT);
			
				break;
					 			
			case GUI_ID_BUTTON5:   //T-
				
				text8 = WM_GetDialogItem(hWin1, GUI_ID_TEXT8);
				Temperature=Temperature-0.05;			
				if(Temperature>=50)
				{
				  Temperature=50;
				}
				else if(Temperature<=0)
				{
					Temperature=0;
				}
				else
				{

			  }
			  SetTemperature(Temperature);
				
				FlashProcess();
				
        if(Temperature>=25)
				{
			    F2S(Temperature, SET_STRT,2);
				}
        else
				{
			    F2S(Temperature+0.000000001, SET_STRT,2);
				}
			  TEXT_SetText(text8,SET_STRT);
			
			
				break;
				 
      case GUI_ID_BUTTON6:   //I+
				
				text10 = WM_GetDialogItem(hWin1, GUI_ID_TEXT10);
				BiasCurrent=BiasCurrent+0.5;			
			  if(BiasCurrent>=180)
				{
				  BiasCurrent=180;
				}
				else if(BiasCurrent<=0)
				{
					BiasCurrent=0;
				}
				else
				{

			  }	
				
  			SetCurrent(BiasCurrent, ModulateCurrent);
				
				FlashProcess();
				
				F2S(BiasCurrent, SET_STRI,1);
				TEXT_SetText(text10,SET_STRI);


				break;
				
      case GUI_ID_BUTTON7: //I-
				
			  text10 = WM_GetDialogItem(hWin1, GUI_ID_TEXT10);
				BiasCurrent=BiasCurrent-0.5;			
			  if(BiasCurrent>=180)
				{
				  BiasCurrent=180;
				}
				else if(BiasCurrent<=0)
				{
					BiasCurrent=0;
				}
				else
				{

			  }	
				
  			SetCurrent(BiasCurrent, ModulateCurrent);
				
				FlashProcess();
				
				F2S(BiasCurrent, SET_STRI,1);
				TEXT_SetText(text10,SET_STRI);
				
		    break;
				
		 case GUI_ID_BUTTON8: //AUTOY
			 
			 hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON8);		
			 if(AUTOY==0)
							{
								 AUTOY=1;								 
								 BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
								 BUTTON_SetText(hButton,"AUTOY");
							}
				else
						 {
								 AUTOY=0;								 
								 BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_RED);
								 BUTTON_SetText(hButton,"FIXY");
							}	
				
		    break;
							
		 case GUI_ID_BUTTON9: //Calibration

		   for(i=0;i<1000;i++)
			 {
				 CalibratedValue[i]=PD_ADCValue[i];
			 }

		    break;			
			 
		case GUI_ID_BUTTON10: //Clear Calibrated data 
		 
		   for(i=0;i<1000;i++)
			 {
				 CalibratedValue[i]=1;
			 }

		    break;	
			
			default:
          break;
		}
 
}


void SwitchLaser(int State)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	
	if(State==1)
	{
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
    GPIO_ResetBits(GPIOB, GPIO_Pin_7);		
		
	}

  else
	 {	 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_SetBits(GPIOB, GPIO_Pin_7);	

		 
	 }

}

void SwitchTEC(int State)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	
	if(State==1)
	{
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
    GPIO_ResetBits(GPIOD, GPIO_Pin_6);		
		
	}

  else
	 {	 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		GPIO_SetBits(GPIOD, GPIO_Pin_6);	

		 
	 }

}

void LaserProtect(WM_HWIN hDlg)
{
	double I=0,T=0,VLD=0;
	BUTTON_Handle hButton;
	
	I=CalCurrent(ADC_ResultValue[2]); //Protect Laser Current
	     if(I>190)
				{
					hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);	
					SwitchLaser(0);								 
				  BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
					BUTTON_SetText(hButton,"LASER OFF");				

					hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON1);	
					SwitchTEC(0);								 
					BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
				  BUTTON_SetText(hButton,"TEC OFF");					
				}
				
	T=CalTemp(ADC_ResultValue[3]);  //Protect Laser Temperature
				if(T>50|T<0)
				{
					hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON1);	
					SwitchTEC(0);								 
					BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
				  BUTTON_SetText(hButton,"TEC OFF");
					
					hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);	
					SwitchLaser(0);								 
				  BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
					BUTTON_SetText(hButton,"LASER OFF");					
				}	
				
	VLD=CalVLD(ADC_ResultValue[5]); //Proetect Laser Voltage
				if(VLD>1.8)
				{
					hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON1);	
					SwitchTEC(0);								 
					BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
				  BUTTON_SetText(hButton,"TEC OFF");
					
					hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);	
					SwitchLaser(0);								 
				  BUTTON_SetBkColor(hButton,BUTTON_CI_UNPRESSED,GUI_GRAY);
					BUTTON_SetText(hButton,"LASER OFF");					
				}	
				
	
}

/****************************************************************************
* ��    �ƣ�void Fun(void)
* ��    �ܣ���ʾ���������
* ��ڲ�������
* ���ڲ�������
* ˵    ����
* ���÷������� 
****************************************************************************/
void Fun(void) {   

	WM_HWIN hGraph;
	int i;
	
	char ADC_STRSETT[8];
	char ADC_STRSETI[8];
	char ADC_STRSETMOD[8];
	
  /* �����Ի���ʱ����������Դ�б���Դ��Ŀ�� */    
  //����1������
  hWin1 = GUI_CreateDialogBox(aDialogCreate1, GUI_COUNTOF(aDialogCreate1), _cbCallback, 0, 0, 0);
	
	//���ô�������  
  FRAMEWIN_SetFont(hWin1,&GUI_Font24B_1);
 
	//Sets the text alignment of the title bar
	FRAMEWIN_SetTextAlign(hWin1,GUI_TA_HCENTER); 
	
	
  //�����ı�����  
  text12 = WM_GetDialogItem(hWin1, GUI_ID_TEXT12);
	text13 = WM_GetDialogItem(hWin1, GUI_ID_TEXT13);
	text11 = WM_GetDialogItem(hWin1, GUI_ID_TEXT11);
	text9 = WM_GetDialogItem(hWin1, GUI_ID_TEXT9);
	text14 = WM_GetDialogItem(hWin1, GUI_ID_TEXT14);
	
  text8 = WM_GetDialogItem(hWin1, GUI_ID_TEXT8);
	text10 = WM_GetDialogItem(hWin1, GUI_ID_TEXT10);
	text15 = WM_GetDialogItem(hWin1, GUI_ID_TEXT15);
	
  text16 = WM_GetDialogItem(hWin1, GUI_ID_TEXT16);
	text17 = WM_GetDialogItem(hWin1, GUI_ID_TEXT17);
	text18 = WM_GetDialogItem(hWin1, GUI_ID_TEXT18);
	text19 = WM_GetDialogItem(hWin1, GUI_ID_TEXT19);
	
	F2S(Temperature, ADC_STRSETT,2);	
	F2S(BiasCurrent, ADC_STRSETI,2);
	F2S(ModulateCurrent, ADC_STRSETMOD,2);

	TEXT_SetText(text8,ADC_STRSETT);	  
	TEXT_SetText(text10,ADC_STRSETI);  
	TEXT_SetText(text15,ADC_STRSETMOD);

	TEXT_SetFont(text16,&GUI_Font16B_ASCII);  
	TEXT_SetFont(text17,&GUI_Font16B_ASCII);
	TEXT_SetFont(text18,&GUI_Font16B_ASCII);  
	TEXT_SetFont(text19,&GUI_Font16B_ASCII);
	
  //Set Button Property
	Button0 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON0);
	BUTTON_SetFont(Button0,&GUI_Font13B_ASCII);
	Button1 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON1);
	BUTTON_SetFont(Button1,&GUI_Font13B_ASCII);
	Button2 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON2);
	BUTTON_SetFont(Button2,&GUI_Font13B_ASCII);
	Button3 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON3);
	BUTTON_SetFont(Button3,&GUI_Font13B_ASCII);
	Button4 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON4);
	BUTTON_SetFont(Button4,&GUI_Font13B_ASCII);
	Button5 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON5);
	BUTTON_SetFont(Button5,&GUI_Font13B_ASCII);
	Button6 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON6);
	BUTTON_SetFont(Button6,&GUI_Font13B_ASCII);
	Button7 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON7);
	BUTTON_SetFont(Button7,&GUI_Font13B_ASCII);
	Button8 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON8);
	BUTTON_SetFont(Button8,&GUI_Font13B_ASCII);
	Button9 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON9);
	BUTTON_SetFont(Button9,&GUI_Font13B_ASCII);
	Button10 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON10);
	BUTTON_SetFont(Button10,&GUI_Font13B_ASCII);
	Button11 = WM_GetDialogItem(hWin1, GUI_ID_BUTTON11);
	BUTTON_SetFont(Button11,&GUI_Font13B_ASCII);
		
	FRAMEWIN_SetActive(hWin1,1);

	hGraph = WM_GetDialogItem(hWin1, GUI_ID_GRAPH0);	
  hDataYT = GRAPH_DATA_YT_Create(GUI_RED, 320, 0, 0);	
  GRAPH_AttachData(hGraph, hDataYT);	
	
  while (1)
  {	

  	if(ADC_R==1)
			{                       
				ADC_R=0;
				//�ı�����ʾ
				TEXT_SetText(text12,ADC_STR1);	  
				TEXT_SetText(text13,ADC_STR2);  
				TEXT_SetText(text11,ADC_STR3);	  
				TEXT_SetText(text9, ADC_STR4);
				TEXT_SetText(text14,ADC_STR6);	  
	
				UpdatePDValues(); 

				
				for (i = 0; i < 320; i++)
				{
				  GRAPH_DATA_YT_AddValue(hDataYT, _aValue[i]);
				}
				
				PeakX=FindPeakX(_aValue,320); //Find PeakX Position
				F2S(PeakX, ADC_STR7,0);	
				TEXT_SetText(text19,ADC_STR7);	
				
				
        LaserProtect(hWin1);

    	}		 

	  WM_Exec(); 							  //ˢ����Ļ
		
  }

}
/******************* (C) COPYRIGHT 2011 �ܶ�STM32 *****END OF FILE****/
