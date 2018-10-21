#ifndef _FLASH_H
#define _FLASH_H

void EraseAllFlash(void);
void StoreIntoFlash(void);
void ReadFlashData(u32 ReadAddress,u16 Length);

#endif /*_FLASH_H*/
