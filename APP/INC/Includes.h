#ifndef _INCLUDES_H
#define _INCLUDES_H

#include    <math.h>
#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <stdarg.h>

#include  <stm32f10x_conf.h>
#include  <stm32f10x.h>

#include "cpal_i2c.h"
#include "Data_Poll.h"
#include "GPIO.h"
#include "ADC.h"
#include "WatchDog.h"
#include "Flash.h"
#include "Fun.h"
#include "DAC.h"

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"

#include  "app_cfg.h"
#include  "os_cfg.h"

#include  "..\uCOS-II\uC-CPU\cpu.h"
#include  "..\uCOS-II\uC-CPU\cpu_def.h"

#include  "..\uCOS-II\Ports\os_cpu.h"
#include  "..\uCOS-II\Source\ucos_ii.h"

#include  "..\BSP\bsp.h"
#include  "..\Touch\ads7843.h"
#include  "..\Touch\calibrate.h"

#include  "..\uCGUIConfig\GUIConf.h"
#include  "..\uCGUIConfig\GUITouchConf.h"
#include  "..\uCGUIConfig\LCDConf.h"

#include "..\GUIinc\GUI.h"
#include "..\GUIinc\lcd.h"
#include "..\GUIinc\GUI_X.h"

#include "..\GUIinc\GUIType.h"

#include "..\GUI_GRAPH\GRAPH.h" 
#include "..\GUI_GRAPH\GRAPH_Private.h" 

#endif

