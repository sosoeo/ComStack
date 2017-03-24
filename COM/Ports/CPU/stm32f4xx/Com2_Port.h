/*
************************************************************************************************************************************
*                      Port/hardware/environment related  files for COM communication module
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     Pin assignment                        |
*          +-----------------------------+---------------+-------------+
*          |  STM32F4xx USART2           |    SN75176    |    Pin      |
*          +-----------------------------+---------------+-------------+
*          |                 GPIO/  PA1  |   T/R selet   |  datasheet  |
*          |                  TXD/  PA2  |          Tx   |  datasheet  |
*          |                  RXD/  PA3  |          Rx   |  datasheet  |
*          +-----------------------------+---------------+-------------+  
*filename     : Comx_Port.h  (where x can be 1 to 10)
*brief        : This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.4
*date         : 2012-09-20
************************************************************************************************************************************
*/

#ifndef _COM2_PORT_H_
#define _COM2_PORT_H_


#include <ComCfg.h>
#include <stm32f4xx.h>

/*
************************************************************************************************************************************
************************************************************************************************************************************
*      Hardware related define
* NOTE:                      MULTIPROCESSOR COMMUNICATION MODE  
*   here we asume this: if the MCU is COM2_MULTIPROCESSOR_ENABLEed, only first byte of every frame produce a interrupt. 
*                           later bytes in this frame will be discarded untill a new frame comming. 
*                           if the MCU is COM2_MULTIPROCESSOR_DISABLE,  any bytes produce a interrupt.
*   1.to use multiprocessor communication(only the intended slave can receive full message contents,thus reducing
*       overhead for all non addressed receivers):
*       WE ONLY NEED DO THIS: after first byte of every frame is received, if it's the intended slave address,
*                             do COM2_MULTIPROCESSOR_DISABLE/clear flag, else do COM2_MULTIPROCESSOR_ENABLE/set flag.
*  2.not use multiprocessor communication
*       WE ONLY NEED DO THIS: 
*                            define COM2_MULTIPROCESSOR_ENABLE   as   none  
*                            define COM2_MULTIPROCESSOR_DISABLE  as   none
*     
*  3.some MCU will need more time delay between  two frame(for example,50ms,STM32 idle line detection mode,it's a little 
*    different with troditional MULTIPROCESSOR COMMUNICATION MODE like 8051.) So if COM is used in POINT to POINT 
*    commnication,Don't use it (get no unnessary delay ).
************************************************************************************************************************************
************************************************************************************************************************************
*/
#define COM2_TRANSPORT_LINE_TX              GPIO_SetBits(GPIOA, GPIO_Pin_1);
#define COM2_TRANSPORT_LINE_RX              GPIO_ResetBits(GPIOA, GPIO_Pin_1);
#define COM2_MULTIPROCESSOR_ENABLE          /*USART_ReceiverWakeUpCmd(USART2,ENABLE)*/
#define COM2_MULTIPROCESSOR_DISABLE         /*USART_ReceiverWakeUpCmd(USART2,DISABLE)*/

                                            /* Place your implementation of fputc here                                */
#define COM2_FPUTC(c)                       USART_SendData(USART2, (INT08U) c);\
                                            while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}

#define COM2_RX_INTERRUPT_ENABLE            USART_ITConfig(USART2,USART_IT_RXNE,ENABLE)
#define COM2_RX_INTERRUPT_DISABLE           USART_ITConfig(USART2,USART_IT_RXNE,DISABLE)
#define COM2_RX_IRQ_FLAG                    (USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)
#define COM2_RX_BYTE_READ(result)           result=USART_ReceiveData(USART2)
#define COM2_RX_IRQ_FLAG_CLEAR              USART_ClearFlag(USART2,USART_FLAG_RXNE)

#define COM2_TIM_IRQ_FLAG_CLEAR             TIM_ClearITPendingBit(TIM7,TIM_FLAG_Update)
#define COM2_TIM_ENABLE                     TIM_Cmd(TIM7, ENABLE)
#define COM2_TIM_DISABLE                    TIM_Cmd(TIM7, DISABLE)
#define COM2_TIM_SET_COUNTER                TIM_SetCounter(TIM7, 0)

/*
************************************************************************************************************************************
*      provided low level  functions for COM service.
************************************************************************************************************************************
*/

void Com2PortInit(Format format, Baud baud,INT08U msOfperiod);

void Com2RxTerminateSignal(void);    /* Post/Mark a flag to indicate that a message receiving have got a result/return*/
void Com2WaitRxTerminate(void);      /* Pend/Wait a flag which indicate that a message receiving overed,AND,clear flag*/

/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif
