#include "Includes.h"

/* Private define ------------------------------------------------------------*/
#define StartAddr  ((u32)0x0807FFF8)
#define EndAddr    ((u32)0x08080000)
#define FLASH_PAGE_SIZE    ((u16)0x08)

u32 EraseCounter = 0x00, Address = 0x00;
vu32 NbrOfPage = 0x00;
volatile FLASH_Status FLASHStatus;

u8  FlashData[8];
int FlashDataIndex=0;		   

void EraseAllFlash(void)
{
  FLASHStatus = FLASH_COMPLETE;
  /* Unlock the Flash Program Erase controller */
  FLASH_Unlock();

  IWDG_ReloadCounter();

  /* Define the number of page to be erased */
  NbrOfPage = (EndAddr - StartAddr) / FLASH_PAGE_SIZE;

  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

  /* Erase the FLASH pages */
  for(EraseCounter = 0; (EraseCounter < NbrOfPage)&& (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
  {	     
    FLASHStatus = FLASH_ErasePage(StartAddr + (FLASH_PAGE_SIZE * EraseCounter)); 
	  IWDG_ReloadCounter(); 
  }
  
  FLASH_Lock();
}

void StoreIntoFlash(void) 
{ 
  u16 DataStore;
  int PageNum; 
  int i=0;
	u8 Data;
  FLASHStatus = FLASH_COMPLETE;
  PageNum=0;	 //Flash pages

  /* Unlock the Flash Program Erase controller */
  FLASH_Unlock();

  IWDG_ReloadCounter();

  /* Define the number of page to be erased */
  NbrOfPage = (EndAddr - StartAddr) / FLASH_PAGE_SIZE;

  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

  /* Erase the FLASH pages */
  Address = StartAddr+PageNum*FLASH_PAGE_SIZE; 

  FLASHStatus = FLASH_ErasePage(Address);
  
  while(!(FLASHStatus == FLASH_COMPLETE));


  while((i<FlashDataIndex)&&(FLASHStatus == FLASH_COMPLETE))
  {	
    IWDG_ReloadCounter();    
    DataStore=FlashData[i];
    Data=FlashData[i+1];
    DataStore=(Data<<8)+DataStore;
    FLASHStatus = FLASH_ProgramHalfWord(Address,DataStore); 
    Address = Address + 2;
    i=i+2;
  }
  
  FlashDataIndex=0;
  memset(FlashData, 0x00, 2);

  FLASH_Lock();	 
}

void ReadFlashData(u32 ReadAddress,u16 Length)
{
  int i=0;
  u8 Data;

  FlashDataIndex=0;
  memset(FlashData, 0x00, 8);

  for(i=0;i<Length;i++)
  {
    Data=*(vu8*)ReadAddress;
	  FlashData[i]=Data;	
    ReadAddress++;
	  FlashDataIndex=i+4;
  }
}













