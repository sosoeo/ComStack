/*
************************************************************************************************************************************
*                      Port/hardware/environment related  files for COM communication module.
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     Pin assignment                        |
*          +-----------------------------+---------------+-------------+
*          |  STM32F2xx USART1,TIM2      |ILX3232        |    Pin      |
*          |  DMA2,Stream7,channel4      |               |             |
*          +-----------------------------+---------------+-------------+
*          |                  TXD/  PA9  |   datasheet   |  datasheet  |
*          |                  RXD/  PA10 |   datasheet   |  datasheet  |
*          +-----------------------------+---------------+-------------+  
*filename     : Comx_Port.h  (where x can be 1 to 10)
*brief        : This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.80
*date         : 2016-04-16
************************************************************************************************************************************
*/

#include "ComCfg.h"

#if USE_COM1

#include "ComType.h"
#include "stm32f2xx.h"

/*
************************************************************************************************************************************
*                                  LOCAL VARIABLE  DEFINE
************************************************************************************************************************************
*/

/*
************************************************************************************************************************************
*                                  LOCAL FUNCTIONS  DECLARE
************************************************************************************************************************************
*/

static void UART_HW_Init(Format format, Baud baud);
static void Timerbase_HW_Init(T32U msOfperiod);

/*
************************************************************************************************************************************
*Description :Initial the U(S)ART to the target config state.Set frame format and baudrate 
*Arguments   :refer to ComInit() description.
*Returns     :none.
*Callers     :ComInit() in COM service.
*Notes       :refer to ComInit() description.
************************************************************************************************************************************
*/

void Com1HWPortInit(Format format, Baud baud,T32U msOfperiod)
 {
    UART_HW_Init(format,baud);
    Timerbase_HW_Init(msOfperiod);
 }


#if USE_DMA_TX_COM1
/*
************************************************************************************************************************************
*Description :Initial the U(S)ART to Send a message by DMA and DMA's ISR,the ISR is used to tell the data sending success 
             :flag&Switch TX/RX driction in RS485.
*Arguments   :data(buffer,size) to be send.
*Returns     :none.
*Notes       :none.
************************************************************************************************************************************
*/
static void UART_Tx_Message_by_DMA(T08U *buffer,T32U size); /* DMA data sending */
    
void Com1TxMessagebyDMA(T08U *buffer,T32U size)/* DMA data sending */
{
    UART_Tx_Message_by_DMA(buffer,size);    
}
#endif
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

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE);           /* 1.Enable GPIO(Tx,Rx pins) clock               */
   /*  ---pin not used in this application--- */                     /* 2.Enable  GPIO(RS485 TxRxDirection pin) clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);            /* 3.Enable USART clock                          */
#if USE_DMA_TX_COM1
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);             /* *4.clock for USART1 DMA2 channel4,Stream7     */
#endif    
/*-----------NVIC-----------------------------------------------------------------------------------------------------*/
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;         
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);                                  /* 4.Enable the USART global Interrupt           */
/*-----------GPIO-----------------------------------------------------------------------------------------------------*/

    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;                      /* 5.Init GPIO Tx                                */
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;                     /* 6.Init GPIO Rx                                */
    GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* --- pin not used in this application --- */                     /* 7.Init GPIO T/R select                        */
  
/*-----------UART-----------------------------------------------------------------------------------------------------*/  
    USART_InitStructure.USART_BaudRate              = (T32U)baud;              
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



static void Timerbase_HW_Init(T32U msOfperiod)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
 
/*-----------RCC------------------------------------------------------------------------------------------------------*/  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);                        /* 1. Enable TIM  clock               */
/*-----------NVIC-----------------------------------------------------------------------------------------------------*/  
    NVIC_InitStructure.NVIC_IRQChannel                      = TIM2_IRQn;
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
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);            /* 3.Set Timbase period accoding baudrate                */

  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);                       
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                 /* 4.Enable Timebase IT interrupt(clear before do it)    */

  TIM_Cmd(TIM2, DISABLE);                                    /* 5.DISABLE the TIM,(the core will hangle it self )     */            
}

#if USE_DMA_TX_COM1
static void UART_Tx_Message_by_DMA(T08U *buffer,T32U size) /* DMA data sending */
{
    DMA_InitTypeDef  DMA_InitStructure;    
    /* Configure DMA controller to manage USART TX and RX DMA request ----------*/  
    DMA_InitStructure.DMA_PeripheralBaseAddr = ((uint32_t)USART1 + 0x04) ; /* USARTx_DR_ADDRESS */
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    /* Here only the unchanged parameters of the DMA initialization structure are
       configured. During the program operation, the DMA will be configured with 
       different parameters according to the operation phase */
         
    DMA_DeInit(DMA2_Stream7);
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;  
    
    /****************** USART will Transmit Specific Command ******************/ 
    /* Prepare the DMA to transfer the transaction command (2bytes) from the
       memory to the USART */  
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)buffer;
    DMA_InitStructure.DMA_BufferSize = size;
    DMA_Init(DMA2_Stream7,&DMA_InitStructure); 

    /* Enable the USART DMA requests */
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    /* Enable the DMA TX Stream, USART will start sending the command code (2bytes) */
    DMA_Cmd(DMA2_Stream7, ENABLE);
      /* /\* Wait the USART DMA Tx transfer complete or time out *\/ */
      /* TimeOut = USER_TIMEOUT;  */
      /* while ((DMA_GetFlagStatus(USARTx_TX_DMA_STREAM, USARTx_TX_DMA_FLAG_TCIF) == RESET)&&(TimeOut != 0)) */
      /* { */
      /* } */
      
      /* if(TimeOut == 0) */
      /* { */
      /*   TimeOut_UserCallback(); */
      /* }  */
      
      /* /\* The software must wait until TC=1. The TC flag remains cleared during all data */
      /*    transfers and it is set by hardware at the last frame閹?end of transmission*\/ */
      /* TimeOut = USER_TIMEOUT; */
      /* while ((USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET)&&(TimeOut != 0)) */
      /* { */
      /* } */
      /* if(TimeOut == 0) */
      /* { */
      /*   TimeOut_UserCallback(); */
      /* }       */
      
      /* /\* Clear DMA Streams flags *\/ */
      /* DMA_ClearFlag(USARTx_TX_DMA_STREAM, USARTx_TX_DMA_FLAG_HTIF | USARTx_TX_DMA_FLAG_TCIF);                                     */
                                          
      /* /\* Disable the DMA Streams *\/ */
      /* DMA_Cmd(USARTx_TX_DMA_STREAM, DISABLE); */
      
      /* /\* Disable the USART Tx DMA request *\/ */
      /* USART_DMACmd(USARTx, USART_DMAReq_Tx, DISABLE); */
}
#endif	/* USE_DMA_TX_COM1 */
/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif /*  USE_COM1 */
