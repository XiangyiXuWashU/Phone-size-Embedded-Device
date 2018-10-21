#ifndef _DATA_POLL_H
#define _DATA_POLL_H

#define  BUFF_MAX  2000

typedef struct{
    unsigned short	     front;
    unsigned short	     rear;
    unsigned char	     uart_buff[BUFF_MAX];
}SEQUEUE;

#define  SLAVE_HEAD1      0x20
#define  SLAVE_HEAD2      0X21
#define  SLAVE_HEAD3      0x22

#define RECV_HEAD_MAX  2 	//Define the length of the structure header of the received data
#define RECV_DATA_MAX  2000	//Define the length of the structure data of the received data
#define UART_BUFFERSIZE 2000	//Define the maximum length of the serial port cache

typedef struct//Define the structure that receives the data, including the header and data and the end
{
    unsigned char  Rhd[RECV_HEAD_MAX];	   //Data header: command dock and command length bits and data length bits
    unsigned char  Rda[RECV_DATA_MAX];	   //data
    unsigned char  Rend[2];	           //Check digit and terminator
}Recv;

void USART_Send_Begin(u8* buff,u16 count);
void InitData(void);
void RecvDepackage(Recv *ComFrame,unsigned char Rbuffer[]);
void Process_cmd(void);
void Data_Poll(unsigned char Check_ok);

double CalVS(int ADCValue);
double CalVMCU(int ADCValue);
double CalCurrent(int ADCValue);
double CalTemp(int ADCValue);
double CalVLD(int ADCValue);

double mWtoDBM(double PmW);
char *F2S(double d, char* str,int digital);

#endif /*_DATA_POLL_H*/
