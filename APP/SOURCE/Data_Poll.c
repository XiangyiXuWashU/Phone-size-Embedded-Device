#include "Includes.h"

SEQUEUE RX;
SEQUEUE TX;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


Recv ComFrame;        //Receive structure, store the received data
unsigned char Comcode;//save command code pointer

u8  Flag_head = 0;   //Judge the USART1 frame header flag
u8  Flag_rx_len = 0; //Receive frame length flag
u16 Slave_length =0; //data length
u8  Slave_check = 0; //checksum
u8  RxBuffer[64];    //Receive data
u16 RxCounter;       //Receive counter


OS_EVENT* ADC_SEM;
volatile unsigned  short int  ADC_TIMEOUT;
volatile unsigned char ADC_R;

extern  const unsigned int Resistance[23];

extern  volatile unsigned  short int FinalY[1000];

void InitData(void)
{
  RX.front = RX.rear = 0;
  TX.front = TX.rear = 0;
}	

/********************************************************************
Function function: queue enqueue, receive data
Entry parameters:
Return:
Remarks:
********************************************************************/
u8 rx_enqueue(u8 x)             
{
 if(RX.front == (RX.rear+1)%BUFF_MAX)
    return 0;
 else
    {
        RX.uart_buff[RX.rear] = x;
        RX.rear = (RX.rear+1)%BUFF_MAX;
        return 0;
    }
}

/********************************************************************
Function: queue dequeue, receive data
Entry parameters:
Return:
Remarks: When dequeuing, first judge the length of L, whether it is greater than 0. Otherwise, you can't leave the team.
********************************************************************/
u8 rx_dequeue(void)   
{
    u8 temp;

    if(RX.front == RX.rear)
     return 0;
     else
     {
        temp = RX.uart_buff[RX.front];
        RX.front = (RX.front+1)%BUFF_MAX;

        return temp;
     }
    }
/********************************************************************
Function function: queue enqueue, send buffer
Entry parameters:
Return:
Remarks:
********************************************************************/
u8 tx_enqueue(u8 x)
{
    if(TX.front == (TX.rear+1)%BUFF_MAX)
    return 0;
    else
    {
        TX.uart_buff[TX.rear] = x;
        TX.rear = (TX.rear+1)%BUFF_MAX;
        return 1;
    }
}
/********************************************************************
Function: queue dequeue, send buffer
Entry parameters:
Return:
Remarks:
********************************************************************/
u8 tx_dequeue(void)
{
     u8 temp;
     if(TX.front == TX.rear)
        return 0;
     else
     {
      temp = TX.uart_buff[TX.front];
      TX.front = (TX.front+1)%BUFF_MAX;

      return temp;
     }
}

//USB Send
void USART_Send_Begin(u8* buff,u16 count)
{
    u32 remain = count;
    u32 index = 0;

    IWDG_ReloadCounter();

    while (remain > 63) //Send up to 64 bytes at a time
    {
        while(GetEPTxStatus(ENDP1)!=EP_TX_NAK);

        UserToPMABufferCopy(&buff[index], ENDP1_TXADDR, 64);

        SetEPTxCount(ENDP1, 64);

        SetEPTxValid(ENDP1);

        index += 64; //Calculate the offset and the number of remaining bytes

        remain -= 64;
    }

    while(GetEPTxStatus(ENDP1)!=EP_TX_NAK);

    //Send the last <=64 bytes of data

    UserToPMABufferCopy(&buff[index], ENDP1_TXADDR, remain);

    SetEPTxCount(ENDP1, remain);

    SetEPTxValid(ENDP1);
}

/********************************************************************
// Function: This function will store the received data into the receiving structure
// Input: Rbuffer - store the received data
// Output: ComFrame - the structure of the receiving data store
********************************************************************/
void RecvDepackage(Recv *ComFrame,unsigned char Rbuffer[])
{
   int i,j;
   ComFrame->Rhd[0]=Rbuffer[0];
   ComFrame->Rhd[1]=Rbuffer[1];

   j=ComFrame->Rhd[1]-4;	         //Receive data byte length

   for(i=0;i<j;i++)	                 //Receive data
   {
      ComFrame->Rda[i]=Rbuffer[i+2];  
   }
   ComFrame->Rend[0]=Rbuffer[j+2];	 //Receive check code
   ComFrame->Rend[1]=Rbuffer[j+3];       //Receive end code
}

/********************************************************************
Function: Unpacking the data in the queue
Entry parameters:
Return:
Remarks: Rxbuffer
********************************************************************/
void Process_cmd(void)
{
   int i=0;
   u8 Data;
   u8 Check_ok;

   if(RX.front != RX.rear)
   {
       Data = rx_dequeue();

       if(!Flag_head)
          {     
           if((Data == SLAVE_HEAD1)||(Data == SLAVE_HEAD2)||(Data == SLAVE_HEAD3))
            {
              RxCounter = 0;
              for(i=0;i<UART_BUFFERSIZE;i++){RxBuffer[i]='\0'; }
              RxBuffer[RxCounter++] = Data;
              Flag_head = 1;
              Flag_rx_len = 0;
              Slave_check=Data;
            }
          }	    
       else if(!Flag_rx_len)
        {
            RxBuffer[RxCounter++] = Data;
            Flag_rx_len = 1;
            Slave_length = Data;                  
            Slave_check^=Data;
        }
       else 	
        {
            RxBuffer[RxCounter++] = Data;

            if(RxCounter <= (Slave_length-2))
                {
                  Slave_check^=Data;
                }

            if(RxCounter == (Slave_length-1))
                {
                   Slave_check=~Slave_check;
                }

            if(RxCounter == Slave_length)
            {
                RxCounter=0;

               if((RxBuffer[Slave_length -2] == Slave_check)&&(RxBuffer[Slave_length -1]==0x00))
                {
                  Check_ok = 1;
                  Data_Poll(Check_ok);
                }

                Flag_head = 0;
                Flag_rx_len = 1;
             }
            if(RxCounter > Slave_length)
               {
                 RxCounter=0;
                 Flag_head = 0;
                 Flag_rx_len = 1;
                }
        }    
     }
}

//////////////////////////////////////////
//////////////////////////////////////////

unsigned char addr;	
unsigned char Data[300];
u8 Buff_temp[2000];


void Data_Poll(unsigned char Check_ok)
{  	
  unsigned char Datalength;
  unsigned char slave_addr;	
  int i=0;

  if(Check_ok)
  {
    RecvDepackage(&ComFrame,RxBuffer);

    Comcode=ComFrame.Rhd[0];
    Datalength=ComFrame.Rhd[1]-6;
    slave_addr = ComFrame.Rda[0];
    addr = ComFrame.Rda[1];
    for(i=0;i<Datalength;i++)
    {
            Data[i] = ComFrame.Rda[i+2];
    }

    if(Comcode==0x20)
    {

    }

    else if(Comcode==0x21)
    {

    }

    else if(Comcode==0x22)	     //Read Transmission Spectrum
    {
        switch(slave_addr)
         {
            case(0x00):   //Read Spectrum
                     for (i = 0; i < 1000; i++)
                       {
                        Buff_temp[i*2]=(FinalY[i]>>8)&0xFF;
                        Buff_temp[i*2+1]=FinalY[i]&0xFF;

                        }

                     USART_Send_Begin(Buff_temp,2000); //USB Sent

                     break;

            default:
                     break;
         }
    }
  }
}

double CalVS(int ADCValue)
{
    double Volt;
    Volt = (double)ADCValue*0.003;
    return Volt;
}

double CalVMCU(int ADCValue)
{
    double Volt;
    Volt = (double)ADCValue*0.002;
    return Volt;
}

double CalCurrent(int ADCValue)
{
    double Current;
    Current = (double)(ADCValue-1250)/10.045;
    return Current;
}

double CalTemp(int ADCValue)
{
  double Temp,R;
  int i=0;
  float k=0,b=0;
	
  R = (double)ADCValue*10;
	
  if(R<=682)
    Temp = 100 ;  
  else if(R>=55786)  
    Temp = -10;
  else 
  {
    while(!((R <=  Resistance[i])&&(R> Resistance[i+1]))) 
    {
      i++;       
    }
    
    if(R == Resistance[i])  Temp = i*5-10;
    
    else
    {
      k = (float)(Resistance[i]-Resistance[i+1])/5;
      b = Resistance[i]*(i-1)-(Resistance[i+1]*(i-2)) ; 
      Temp = (b - R)/k;	
		
    }
  } 

  return Temp;
}

double CalVLD(int ADCValue)
{
    double Volt;
    Volt = (double)ADCValue*0.001;
    return Volt;
}

double mWtoDBM(double PmW)  
{
    double PdBm;
    PdBm = 10*log10(PmW);
    return PdBm;
}

char *F2S(double d, char* str,int digital)
{
  char str1[40];
  int j=0,k,i;
  i = (int)d; 
	
  if (i == 0)
  {
     str1[j++] = '0';
  }	
	
  while(i>0)
  {
    str1[j++] = i%10+'0';
    i /= 10;
  }
		
  while(i<0)
  {
    i *= -1;
    str1[j++] = i%10+'0';
    i /= 10;
  }
		
  if (d < 0)
  {
     str1[j++] = '-';

     /* Make the value positive. */
     d *= -1;
  }
		
		
  for(k=0;k<j;k++)
    str[k] = str1[j-1-k]; 

  str[j++] = '.';
  d -= (int)d;
  for(i=0;i<digital;i++)
  {
    d *= 10;	
  	str[j++] = (int)d+'0';
    d -= (int)d;
  }
	

  while(str[--j]=='0'&&j>=0)
	{
		
	}

  str[++j] = '\0';  
	if(str[--j]=='.')
	{
		str[j] = '\0';  
	}
  return str;
}




  

