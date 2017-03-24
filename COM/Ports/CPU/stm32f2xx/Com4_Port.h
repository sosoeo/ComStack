/*
************************************************************************************************************************************
*                      Port/hardware/environment related  files for COM communication module.
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     Pin assignment                        |
*          +-----------------------------+---------------+-------------+
*          |  STM32F2xx USART3 ,TIM4     |ILX3232        |    Pin      |
*          +-----------------------------+---------------+-------------+
*          |                  TXD/  PB10 |   datasheet   |  datasheet  |
*          |                  RXD/  PB11 |   datasheet   |  datasheet  |
*          +-----------------------------+---------------+-------------+  
*filename     : Comx_Port.h  (where x can be 1 to 10)
*brief        : This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.71
*date         : 2014-11-05
************************************************************************************************************************************
*/

#ifndef _COM4_HW_PORT_H_
#define _COM4_HW_PORT_H_

#include "ComCfg.h"

#if USE_COM4

#include "ComType.h"
#include "stm32f2xx.h"

/*
************************************************************************************************************************************
************************************************************************************************************************************
*      Hardware related define
* NOTE:                      MULTIPROCESSOR COMMUNICATION MODE  
*   here we asume this: if the MCU is COM4_MULTIPROCESSOR_ENABLEed, only first byte of every frame produce a interrupt. 
*                           later bytes in this frame will be discarded untill a new frame comming. 
*                           if the MCU is COM4_MULTIPROCESSOR_DISABLE,  any bytes produce a interrupt.
*   1.to use multiprocessor communication(only the intended slave can receive full message contents,thus reducing
*       overhead for all non addressed receivers):
*       WE ONLY NEED DO THIS: after first byte of every frame is received, if it's the intended slave address,
*                             do COM4_MULTIPROCESSOR_DISABLE/clear flag, else do COM4_MULTIPROCESSOR_ENABLE/set flag.
*  2.not use multiprocessor communication
*       WE ONLY NEED DO THIS: 
*                            define COM4_MULTIPROCESSOR_ENABLE   as   none  
*                            define COM4_MULTIPROCESSOR_DISABLE  as   none
*     
*  3.some MCU will need more time delay between  two frame(for example,50ms,STM32 idle line detection mode,it's a little 
*    different with troditional MULTIPROCESSOR COMMUNICATION MODE like 8051.) So if COM is used in POINT to POINT 
*    commnication,Don't use it (get no unnessary delay ).
************************************************************************************************************************************
************************************************************************************************************************************
*/
#define COM4_TRANSPORT_LINE_TX              /* none                                                                   */
#define COM4_TRANSPORT_LINE_RX              /* none                                                                   */
#define COM4_MULTIPROCESSOR_ENABLE          /*USART_ReceiverWakeUpCmd(USART3,ENABLE)                                  */
#define COM4_MULTIPROCESSOR_DISABLE         /*USART_ReceiverWakeUpCmd(USART3,DISABLE)                                 */

                                            /* Place your implementation of fputc here                                */
#define COM4_FPUTC(c)                       USART_SendData(USART3, (T08U) c);\
                                            while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET){}

#define COM4_RX_INTERRUPT_ENABLE            USART_ITConfig(USART3,USART_IT_RXNE,ENABLE)
#define COM4_RX_INTERRUPT_DISABLE           USART_ITConfig(USART3,USART_IT_RXNE,DISABLE)
#define COM4_RX_IRQ_FLAG                    (USART_GetITStatus(USART3,USART_IT_RXNE)!=RESET)
#define COM4_RX_BYTE_READ(result)           result=USART_ReceiveData(USART3)
#define COM4_RX_IRQ_FLAG_CLEAR              USART_ClearFlag(USART3,USART_FLAG_RXNE)

#define COM4_TIM_IRQ_FLAG_CLEAR             TIM_ClearITPendingBit(TIM4,TIM_FLAG_Update)
#define COM4_TIM_ENABLE                     TIM_Cmd(TIM4, ENABLE)
#define COM4_TIM_DISABLE                    TIM_Cmd(TIM4, DISABLE)
#define COM4_TIM_SET_COUNTER                TIM_SetCounter(TIM4, 0)

/*
************************************************************************************************************************************
*      Hardware related  functions
************************************************************************************************************************************
*/

void Com4HWPortInit(Format format, Baud baud,T32U msOfperiod);

#if USE_DMA_TX_COM4
void Com4TxMessagebyDMA(T08U *buffer,T32U size);/* DMA data sending */
#endif	/* USE_DMA_TX_COM4 */

/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif /*  USE_COM4        */
#endif /* _COM4_HW_PORT_H_ */
