/*
************************************************************************************************************************************
*                      Port/hardware/environment related  files for COM communication module.
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     Pin assignment                        |
*          +-----------------------------+---------------+-------------+
*          |  STM32F10x USART1           |ILX3232        |    Pin      |
*          +-----------------------------+---------------+-------------+
*          |                  TXD/  PA9  |   datasheet   |  datasheet  |
*          |                  RXD/  PA10 |   datasheet   |  datasheet  |
*          +-----------------------------+---------------+-------------+  
*filename     : Comx_Port.c  (where x can be 1 to 10)
*brief        : This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.4
*date         : 2012-09-20
************************************************************************************************************************************
*/

#include <ComCfg.h>
#include <stm32f4xx.h>

#if COM_CFG_BLOCKING_USE_OS
#include"includes.h"                            /* for OS signal pend & post service in blocking mode                 */
#endif

/*
************************************************************************************************************************************
*                                  LOCAL VARIABLE  DEFINE
************************************************************************************************************************************
*/
#if COM_CFG_BLOCKING_USE_OS
static  OS_SEM  Com1Sem;                        /*Source access the COM1/UART1 for multi-thread access safety         */ 
static  OS_SEM  Com1SemRxTerminate;             /* Event for Com..Task                                                */
#endif

/*
************************************************************************************************************************************
*                                  LOCAL FUNCTIONS  DECLARE
************************************************************************************************************************************
*/

static void UART_HW_Init(Format format, Baud baud);
static void Timerbase_HW_Init(INT08U msOfperiod);
static void OS_Emulate_Init(void);

/*
************************************************************************************************************************************
*Description :Initial the U(S)ART to the target config state.Set frame format and baudrate 
*Arguments   :refer to ComInit() description.
*Returns     :none.
*Callers     :ComInit() in COM service.
*Notes       :refer to ComInit() description.
************************************************************************************************************************************
*/

void Com1PortInit(Format format, Baud baud,INT08U msOfperiod)
{

    UART_HW_Init(format,baud);
    Timerbase_HW_Init(msOfperiod);
    OS_Emulate_Init();
}


/*
************************************************************************************************************************************
************************************************************************************************************************************
*                RTOS SERVICE PORT
*Description : RX-->Post a signal to tell the system that receiver was terminated.(whitch means
*             a message was received,or timeout happened,or err happened.)
*Arguments   :None
*Returns     :None.
*Callers     :internal.
*Notes       :none.
************************************************************************************************************************************
************************************************************************************************************************************
*/

static void OS_Emulate_Init(void)
{
#if COM_CFG_BLOCKING_USE_OS
    OS_ERR err;  
    OSSemCreate(&Com1Sem,"Com1 Access Sem",1,&err);         /* used by COMTestx Task                 */ 
    OSSemCreate(&Com1SemRxTerminate,"Com1 Rx over",0,&err); /* used by COMTestx Task                 */    
#endif
}



void Com1RxTerminateSignal(void) 
{
#if COM_CFG_BLOCKING_USE_OS
    OS_ERR err; 
    OSSemPost(&Com1SemRxTerminate,OS_OPT_POST_1,&err);
#endif
}

void Com1WaitRxTerminate(void) 
{
#if COM_CFG_BLOCKING_USE_OS
    CPU_TS ts;
    OS_ERR err; 
    OSSemPend(&Com1SemRxTerminate,0,OS_OPT_PEND_BLOCKING, &ts,&err);
#endif
}


/*
************************************************************************************************************************************
************************************************************************************************************************************
*                         STANDARD HARDWARE  INITIALIZATION SERVICE
*Description :Initialization the hardware,This is the lowest level operation. 
*Arguments   :None
*Returns     :None.
*Caller      :Com1PortInit(void).
*Notes       :directly call it in related ISR entry. 
************************************************************************************************************************************
************************************************************************************************************************************
*/

static void UART_HW_Init(Format format, Baud baud)
{

    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
  
/*-----------RCC------------------------------------------------------------------------------------------------------*/

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);            /* 1.Enable GPIO(Tx,Rx pins) clock               */
  /*  ---pin not used in this application--- */                      /* 2.Enable  GPIO(RS485 TxRxDirection pin) clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);            /* 3.Enable USART clock                          */

/*-----------NVIC-----------------------------------------------------------------------------------------------------*/
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;         
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);                                  /* 4.Enable the USART global Interrupt           */
/*-----------GPIO-----------------------------------------------------------------------------------------------------*/                         
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                        /* 5.Init GPIO Tx                                */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                       /* 6.Init GPIO Rx                                */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* RS485 chips TxRxDirection  control line: PB8 */                 /* 7.Init GPIO T/R select                        */

  /* --- pin not used in this application --- */
  
/*-----------UART-----------------------------------------------------------------------------------------------------*/  
    USART_InitStructure.USART_BaudRate              = (INT32U)baud;              
    switch(format) {
    case _8N1:
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_No;     
	break;

    case _8E1:
	USART_InitStructure.USART_WordLength            = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_Even;     
	break;

    case _8O1:
	USART_InitStructure.USART_WordLength            = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_Odd;     
	break;

    case _8N2:
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_2;
	USART_InitStructure.USART_Parity                = USART_Parity_No;     
	break;

    case _7E1:
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_Even;     
	break;

    case _7O1:
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_Odd;     
	break;

    case _7N2:

    default:
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;
	USART_InitStructure.USART_Parity                = USART_Parity_No;     
    }

    USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);                       /* 8. Set baudrate & frame format                 */        
    
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);                 /* 9. Disable Rx&&Tx interrupt                    */          
    USART_Cmd(USART1, ENABLE);                                      /* 10.Enable U(S)ART  peripheral                  */
}



static void Timerbase_HW_Init(INT08U msOfperiod)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
 
/*-----------RCC------------------------------------------------------------------------------------------------------*/  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);                        /* 1. Enable TIM  clock    */
/*-----------NVIC-----------------------------------------------------------------------------------------------------*/  
    NVIC_InitStructure.NVIC_IRQChannel                      = TIM6_DAC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 14;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;           /* 2. Enable TIM IRQ global interrupt */
    NVIC_Init(&NVIC_InitStructure);                                                   

/*-----------TIMER BASE-----------------------------------------------------------------------------------------------*/  
/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
|    Note:                                                                                    |
|     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32fxxx.c file.|
|     Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()       |
|     function to update SystemCoreClock variable value. Otherwise, any configuration         |
|     based on this variable will be incorrect.                                               |
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||  
*/
  TIM_TimeBaseStructure.TIM_Period            = 10*msOfperiod; /* !!!this is correct only at counter pulse 10KHz!!!   */  
  TIM_TimeBaseStructure.TIM_Prescaler         = (uint16_t)(((SystemCoreClock/2) / 10000)-1);     /* MUST(10khz) pulse */ 
  TIM_TimeBaseStructure.TIM_ClockDivision     = 0;
  TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);            /* 3.Set Timbase period accoding baudrate                */

  TIM_ClearITPendingBit(TIM6, TIM_IT_Update);                       
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);                 /* 4.Enable Timebase IT interrupt(clear before do it)    */

  TIM_Cmd(TIM6, DISABLE);                                    /* 5.DISABLE the TIM,(the core will hangle it self )     */            

}


/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
