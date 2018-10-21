#include "includes.h"

extern OS_EVENT* ADC_SEM;
extern volatile unsigned  short int  ADC_TIMEOUT;
extern volatile unsigned  short int  ADC_ConvertedValue[60], ADC_ResultValue[6];
extern volatile unsigned char ADC_R;
extern char ADC_STR1[5];	//ADC1 Integer to Char
extern char ADC_STR2[5];	//ADC1 Integer to Char
extern char ADC_STR3[5];	//ADC1 Integer to Char
extern char ADC_STR4[5];	//ADC1 Integer to Char
extern char ADC_STR5[5];	//ADC1 Integer to Char
extern char ADC_STR6[5];	//ADC1 Integer to Char
extern char ADC_STR7[5];        //Peak X

extern void Fun(void);
/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
static  OS_STK App_TaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_STK AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE];
static  OS_STK AppTaskKbdStk[APP_TASK_KBD_STK_SIZE]; 
static  OS_STK Task_Process_cmdStk[Task_Process_cmd_STK_SIZE];
static  OS_STK Task_AdcStk[Task_Adc_STK_SIZE];
static  OS_STK Task_WDGStk[Task_WDG_STK_SIZE];

static  void App_TaskCreate(void);
static  void App_TaskStart(void* p_arg);
static  void AppTaskUserIF(void* p_arg);
static  void AppTaskKbd(void* p_arg);
static  void Task_Process_cmd(void* p_arg);
static  void Task_ADC(void* p_arg);
static  void Task_WDG(void* p_arg);

/****************************************************************************
* Name: int main(void)
* Function: main function entry
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
CPU_INT08U os_err;  

int main(void)
{	
   /* Prohibit all interruptions */
   CPU_IntDis();
   
   /* ucosII initialization */
   OSInit();                                                  

   /* Hardware platform initialization */
   BSP_Init();                                           
   
   ADC_TIMEOUT=0;
	
   //Create a main task with the highest priority.
   //Another purpose of creating this task is to use statistical tasks in the future.
   os_err = OSTaskCreate((void (*) (void *)) App_TaskStart, //Pointer to the task code
                       (void *) 0, // pointer to the argument passed to the task when the task starts executing
                       (OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1];//The top-of-stack pointer assigned to the stack of the task is decremented from top to bottom
                       (INT8U) APP_TASK_START_PRIO);  //Priority assigned to the task
             
   OSTimeSet(0);       //ucosII beat counter clear 0 beat counter is 0-4294967295
   OSStart();          //Start the ucosII kernel
   return (0);
}

/****************************************************************************
* Name: static void App_TaskStart(void* p_arg)
* Function: Start task creation
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
static  void App_TaskStart(void* p_arg)
{ 
  (void) p_arg;
   //Initialize the ucosII clock beat
   OS_CPU_SysTickInit();
                               
   //Enable the statistical task of ucos
#if (OS_TASK_STAT_EN > 0)   
   OSStatInit();                //Statistical task initialization function
#endif

   App_TaskCreate();		//Establish other tasks

   while (1)
   {		      	  
      OSTimeDlyHMSM(0, 0, 1, 100);
   }
}

/****************************************************************************
* Name: static void App_TaskCreate(void)
* Function: function to build the remaining tasks
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
static  void App_TaskCreate(void)
{
   ADC_SEM=OSSemCreate(0);   //Establish ADC sampling semaphore
   /* Create user interface tasks */
   OSTaskCreateExt(AppTaskUserIF,                  //Pointer to the task code
                    (void *)0,	                   //Pointer to the argument passed to the task when the task begins execution
                    (OS_STK *)&AppTaskUserIFStk[APP_TASK_USER_IF_STK_SIZE-1],
                    APP_TASK_USER_IF_PRIO,         //Priority assigned to the task
                    APP_TASK_USER_IF_PRIO,         //Prepare a special identifier for future versions, with the current version and task priority
                    (OS_STK *)&AppTaskUserIFStk[0],//Pointer to the bottom of the task stack for inspection of the stack
                    APP_TASK_USER_IF_STK_SIZE,     //Specify the capacity of the stack for the inspection of the stack
                    (void *)0,	                   //Pointer to the user-attached data field, used to extend the task's task control block
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);//Option, specify whether to allow stack checking, whether to clear the stack, whether the task wants
                    
   /* Establish touch drive tasks */
   OSTaskCreateExt(AppTaskKbd,
                    (void *)0,
                    (OS_STK *)&AppTaskKbdStk[APP_TASK_KBD_STK_SIZE-1],
                    APP_TASK_KBD_PRIO,
                    APP_TASK_KBD_PRIO,
                    (OS_STK *)&AppTaskKbdStk[0],
                    APP_TASK_KBD_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);	

   /* External communication task */
   OSTaskCreateExt(Task_Process_cmd,
                    (void *)0,
                    (OS_STK *)&Task_Process_cmdStk[Task_Process_cmd_STK_SIZE-1],
                    Task_Process_cmd_PRIO,
                    Task_Process_cmd_PRIO,
                    (OS_STK *)&Task_Process_cmdStk[0],
                    Task_Process_cmd_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);
   
   /* ADC Acquisition task */
   OSTaskCreateExt(Task_ADC,
                    (void *)0,
                    (OS_STK *)&Task_AdcStk[Task_Adc_STK_SIZE-1],
                    Task_Adc_PRIO,
                    Task_Adc_PRIO,
                    (OS_STK *)&Task_AdcStk[0],
                    Task_Adc_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);							

   /* Watchdog task */
   OSTaskCreateExt(Task_WDG,
                    (void *)0,
                    (OS_STK *)&Task_WDGStk[Task_WDG_STK_SIZE-1],
                    Task_WDG_PRIO,
                    Task_WDG_PRIO,
                    (OS_STK *)&Task_WDGStk[0],
                    Task_WDG_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR);										
}

/****************************************************************************
* Name: static void AppTaskUserIF (void *p_arg)
* Function: User interface task
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
static  void  AppTaskUserIF (void *p_arg)
{											   
  (void)p_arg;
  GUI_Init();	         //ucgui init
  while(1) 
  {	
     Fun(); 	         //Interface main program
  }
}

/****************************************************************************
* Name: static void AppTaskKbd (void *p_arg)
* Function: Touch screen coordinate acquisition task
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
static  void  AppTaskKbd (void *p_arg)
{
   (void)p_arg;
   while(1) 
   { 
    /* Delays 10ms will read the touch coordinates */
    OSTimeDlyHMSM(0,0,0,10); 	               
    GUI_TOUCH_Exec();
   }
}
/****************************************************************************
* Name: static void Task_Process_cmd(void* p_arg)
* Function: External communication task
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
static  void Task_Process_cmd(void* p_arg)
{
   (void) p_arg;	    
   while (1)
   {     
    OSTimeDlyHMSM(0, 0, 0, 5);
    Process_cmd();   
   }
}

/****************************************************************************
* Name: static void Task__WDG(void* p_arg)
* Function: Watchdog mission
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
static  void Task_WDG(void* p_arg)
{
   (void) p_arg;	    
   while (1)
   {   
    //watchdog delay 150MS
    OSTimeDlyHMSM(0, 0, 0, 150);
    IWDG_ReloadCounter(); 
   }
}

/****************************************************************************
* Name: static void Task_ADC(void *p_arg)
* Function: ADC processing task
* Entry parameters: none
* Export parameters: none
* Description:
* Call method: None
****************************************************************************/
static  void Task_ADC(void *p_arg)
{ 	
   INT8U err;
   double VS,VMCU,I,T,VLD;
   (void)p_arg;	  
   while(1)
	 {   
            OSSemPend(ADC_SEM,0,&err);  //Wait for the ADC semaphore

            VS=CalVS(ADC_ResultValue[0]);
            VMCU=CalVMCU(ADC_ResultValue[1]);
            I=CalCurrent(ADC_ResultValue[2]);
            T=CalTemp(ADC_ResultValue[3]);
            VLD=CalVLD(ADC_ResultValue[5]);

            if(VS<0.001)
            {
                ADC_STR3[0]=0x30;
                ADC_STR3[1]=0x2E;
                ADC_STR3[2]=0x30;
                ADC_STR3[3]=0x30;
            }
            else
            {
                F2S(VS, ADC_STR1,2);
            }

            if(VMCU<0.001)
            {
                ADC_STR3[0]=0x30;
                ADC_STR3[1]=0x2E;
                ADC_STR3[2]=0x30;
                ADC_STR3[3]=0x30;
            }
            else
            {
                F2S(VMCU, ADC_STR2,2);
            }

            if(I<0.001)
            {
                ADC_STR3[0]=0x30;
                ADC_STR3[1]=0x2E;
                ADC_STR3[2]=0x30;
                ADC_STR3[3]=0x30;
            }
            else
            {
                F2S(I, ADC_STR3,2);
            }

            F2S(T, ADC_STR4,2);

            if(VLD<0.001)
            {
                ADC_STR3[0]=0x30;
                ADC_STR3[1]=0x2E;
                ADC_STR3[2]=0x30;
                ADC_STR3[3]=0x30;
            }
            else
            {
                F2S(VLD, ADC_STR6,2);
            }
    } 
}

#if (OS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument : ptcb   is a pointer to the task control block of the task being created.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskCreateHook(OS_TCB* ptcb)
{
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskDelHook(OS_TCB* ptcb)
{
   (void) ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument : none.
*
* Note     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 251
void App_TaskIdleHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument : none.
*********************************************************************************************************
*/

void App_TaskStatHook(void)
{
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument : none.
*
* Note     : 1 Interrupts are disabled during this call.
*
*            2  It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if OS_TASK_SW_HOOK_EN > 0
void App_TaskSwHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                     OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*               up most of the TCB.
*
* Argument : ptcb    is a pointer to the TCB of the task being created.
*
* Note     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 204
void App_TCBInitHook(OS_TCB* ptcb)
{
   (void) ptcb;
}
#endif

#endif
/******************* (C) COPYRIGHT 2011 �ܶ�STM32 *****END OF FILE****/
