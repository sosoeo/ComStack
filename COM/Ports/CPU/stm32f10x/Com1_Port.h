/*
************************************************************************************************************************************
*                      Port/hardware/environment related  files for COM communication module.
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     Pin assignment                        |
*          +-----------------------------+---------------+-------------+
*          |  STM32F10x USART1,TIM2      |ILX3232        |    Pin      |
*          |  DMA1,Channel4              |               |             |
*          +-----------------------------+---------------+-------------+
*          |                  TXD/  PA9  |   datasheet   |  datasheet  |
*          |                  RXD/  PA10 |   datasheet   |  datasheet  |
*          +-----------------------------+---------------+-------------+  
*filename     : Comx_Port.h  (where x can be 1 to 10)
*brief        : This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.8
*date         : 2016-03-10
************************************************************************************************************************************
*/

#ifndef _COM1_HW_PORT_H_
#define _COM1_HW_PORT_H_

#include "ComCfg.h"

#if USE_COM1

#include "ComType.h"
#include "stm32f10x.h"

/*
************************************************************************************************************************************
************************************************************************************************************************************
*      Hardware related define
* NOTE:                      MULTIPROCESSOR COMMUNICATION MODE  
*   here we asume this: if the MCU is COM1_MULTIPROCESSOR_ENABLEed, only first byte of every frame produce a interrupt. 
*                           later bytes in this frame will be discarded untill a new frame comming. 
*                           if the MCU is COM1_MULTIPROCESSOR_DISABLE,  any bytes produce a interrupt.
*   1.to use multiprocessor communication(only the intended slave can receive full message contents,thus reducing
*       overhead for all non addressed receivers):
*       WE ONLY NEED DO THIS: after first byte of every frame is received, if it's the intended slave address,
*                             do COM1_MULTIPROCESSOR_DISABLE/clear flag, else do COM1_MULTIPROCESSOR_ENABLE/set flag.
*  2.not use multiprocessor communication
*       WE ONLY NEED DO THIS: 
*                            define COM1_MULTIPROCESSOR_ENABLE   as   none  
*                            define COM1_MULTIPROCESSOR_DISABLE  as   none
*     
*  3.some MCU will need more time delay between  two frame(for example,50ms,STM32 idle line detection mode,it's a little 
*    different with troditional MULTIPROCESSOR COMMUNICATION MODE like 8051.) So if COM is used in POINT to POINT 
*    commnication,Don't use it (get no unnessary delay ).
************************************************************************************************************************************
************************************************************************************************************************************
*/
#define COM1_TRANSPORT_LINE_TX              /* none                                                                   */
#define COM1_TRANSPORT_LINE_RX              /* none                                                                   */
#define COM1_MULTIPROCESSOR_ENABLE          /*USART_ReceiverWakeUpCmd(USART1,ENABLE)                                  */
#define COM1_MULTIPROCESSOR_DISABLE         /*USART_ReceiverWakeUpCmd(USART1,DISABLE)                                 */

                                            /* Place your implementation of fputc here                                */
#define COM1_FPUTC(c)                       USART_SendData(USART1, (T08U) c);\
                                            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){}

#define COM1_RX_INTERRUPT_ENABLE            USART_ITConfig(USART1,USART_IT_RXNE,ENABLE)
#define COM1_RX_INTERRUPT_DISABLE           USART_ITConfig(USART1,USART_IT_RXNE,DISABLE)
#define COM1_RX_IRQ_FLAG                    (USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
#define COM1_RX_BYTE_READ(result)           result=USART_ReceiveData(USART1)
#define COM1_RX_IRQ_FLAG_CLEAR              USART_ClearFlag(USART1,USART_FLAG_RXNE)

#define COM1_TIM_IRQ_FLAG_CLEAR             TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update)
#define COM1_TIM_ENABLE                     TIM_Cmd(TIM2, ENABLE)
#define COM1_TIM_DISABLE                    TIM_Cmd(TIM2, DISABLE)
#define COM1_TIM_SET_COUNTER                TIM_SetCounter(TIM2, 0)

#if USE_DMA_TX_COM1
#define COM1_TX_INTERRUPT_ENABLE            USART_ITConfig(USART1,USART_IT_TC,ENABLE)
#define COM1_TX_INTERRUPT_DISABLE           USART_ITConfig(USART1,USART_IT_TC,DISABLE)
#define COM1_TX_IRQ_FLAG                    (USART_GetITStatus(USART1,USART_IT_TC)!=RESET)
#define COM1_TX_IRQ_FLAG_CLEAR              USART_ClearFlag(USART1,USART_IT_TC)
#endif
/*
************************************************************************************************************************************
*      provided low level  functions for COM service.
************************************************************************************************************************************
*/

void Com1HWPortInit(Format format, Baud baud,T32U msOfperiod);

#if USE_DMA_TX_COM1
void Com1TxMessagebyDMA(T08U *buffer,T32U size);/* DMA data sending */
#endif	/* USE_DMA_TX_COM1 */
/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif /*  USE_COM1        */
#endif /* _COM1_HW_PORT_H_ */
