/*
************************************************************************************************************************************
*                      Port/hardware/environment related  files for COM communication module
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     Pin assignment                        |
*          +-----------------------------+---------------+-------------+
*          |  STM32F2xx USART2 ,TIM3     |    SN75176    |    Pin      |
*          |  DMA2,Stream6,channel5      |               |             |
*          +-----------------------------+---------------+-------------+
*          |                  TXD/  PC6  |   datasheet   |  datasheet  |
*          |                  RXD/  PC7  |   datasheet   |  datasheet  |
*          |                  VCC/  3.3V |   VCC3.3/5V   |  datasheet  |
*          |              TXD/RXD/  none | TX/RX Select  |  datasheet  |
*          +-----------------------------+---------------+-------------+  
*filename     : Comx_Port.h  (where x can be 1 to 10)
*brief        : This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.8
*date         : 2016-03-01
************************************************************************************************************************************
*/

#ifndef _COM3_HW_PORT_H_
#define _COM3_HW_PORT_H_


#include "ComCfg.h"

#if USE_COM3

#include "ComType.h"
#include "stm32f2xx.h"

/*
************************************************************************************************************************************
************************************************************************************************************************************
*      Hardware related define
* NOTE:                      MULTIPROCESSOR COMMUNICATION MODE  
*   here we asume this: if the MCU is COM3_MULTIPROCESSOR_ENABLEed, only first byte of every frame produce a interrupt. 
*                           later bytes in this frame will be discarded untill a new frame comming. 
*                           if the MCU is COM3_MULTIPROCESSOR_DISABLE,  any bytes produce a interrupt.
*   1.to use multiprocessor communication(only the intended slave can receive full message contents,thus reducing
*       overhead for all non addressed receivers):
*       WE ONLY NEED DO THIS: after first byte of every frame is received, if it's the intended slave address,
*                             do COM3_MULTIPROCESSOR_DISABLE/clear flag, else do COM3_MULTIPROCESSOR_ENABLE/set flag.
*  2.not use multiprocessor communication
*       WE ONLY NEED DO THIS: 
*                            define COM3_MULTIPROCESSOR_ENABLE   as   none  
*                            define COM3_MULTIPROCESSOR_DISABLE  as   none
*     
*  3.some MCU will need more time delay between  two frame(for example,50ms,STM32 idle line detection mode,it's a little 
*    different with troditional MULTIPROCESSOR COMMUNICATION MODE like 8051.) So if COM is used in POINT to POINT 
*    commnication,Don't use it (get no unnessary delay ).
************************************************************************************************************************************
************************************************************************************************************************************
*/
#define COM3_TRANSPORT_LINE_TX              /*none*/
#define COM3_TRANSPORT_LINE_RX              /*none*/
#define COM3_MULTIPROCESSOR_ENABLE          /*USART_ReceiverWakeUpCmd(USART2,ENABLE)*/
#define COM3_MULTIPROCESSOR_DISABLE         /*USART_ReceiverWakeUpCmd(USART2,DISABLE)*/

                                            /* Place your implementation of fputc here                                */
#define COM3_FPUTC(c)                       USART_SendData(USART6, (T08U) c);\
                                            while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET){}

#define COM3_RX_INTERRUPT_ENABLE            USART_ITConfig(USART6,USART_IT_RXNE,ENABLE)
#define COM3_RX_INTERRUPT_DISABLE           USART_ITConfig(USART6,USART_IT_RXNE,DISABLE)
#define COM3_RX_IRQ_FLAG                    (USART_GetITStatus(USART6,USART_IT_RXNE)!=RESET)
#define COM3_RX_BYTE_READ(result)           result=USART_ReceiveData(USART6)
#define COM3_RX_IRQ_FLAG_CLEAR              USART_ClearFlag(USART6,USART_FLAG_RXNE)

#define COM3_TIM_IRQ_FLAG_CLEAR             TIM_ClearITPendingBit(TIM3,TIM_FLAG_Update)
#define COM3_TIM_ENABLE                     TIM_Cmd(TIM3, ENABLE)
#define COM3_TIM_DISABLE                    TIM_Cmd(TIM3, DISABLE)
#define COM3_TIM_SET_COUNTER                TIM_SetCounter(TIM3, 0)

#if USE_DMA_TX_COM3
#define COM3_TX_INTERRUPT_ENABLE            USART_ITConfig(USART6,USART_IT_TC,ENABLE)
#define COM3_TX_INTERRUPT_DISABLE           USART_ITConfig(USART6,USART_IT_TC,DISABLE)
#define COM3_TX_IRQ_FLAG                    (USART_GetITStatus(USART6,USART_IT_TC)!=RESET)
#define COM3_TX_IRQ_FLAG_CLEAR              USART_ClearFlag(USART6,USART_IT_TC)
#endif
/*
************************************************************************************************************************************
*      provided low level  functions for COM service.
************************************************************************************************************************************
*/

void Com3HWPortInit(Format format, Baud baud,T32U msOfperiod);

#if USE_DMA_TX_COM3
void Com3TxMessagebyDMA(T08U *buffer,T32U size);/* DMA data sending */
#endif	/* USE_DMA_TX_COM3 */

/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif /*  USE_COM3         */
#endif /* _COM3_HW_PORT_H_  */
