/*
************************************************************************************************************************************
*                      Port/hardware/environment related  files for COM communication module.
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     Pin assignment                        |
*          +-----------------------------+---------------+-------------+
*          |  STM32F2xx UART4 ,TIM5      |ILX3232/ILX485 |    Pin      |
*          |  DMA1,Stream4,channel4      |               |             |
*          +-----------------------------+---------------+-------------+
*          |                  TXD/  PC10 |   datasheet   |  datasheet  |
*          |                  RXD/  PC11 |   datasheet   |  datasheet  |
*          |            (GPIO)TR /  PC12 |   datasheet   |  datasheet  |
*          +-----------------------------+---------------+-------------+  
*filename     : Comx_Port.h  (where x can be 1 to 10)
*brief        : This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.80
*date         : 2016-01-12
************************************************************************************************************************************
*/

#ifndef _COM2_HW_PORT_H_
#define _COM2_HW_PORT_H_

#include "ComCfg.h"

#if USE_COM2

#include "ComType.h"
#include "stm32f2xx.h"

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
#define COM2_TRANSPORT_LINE_TX              GPIO_ResetBits(GPIOC, GPIO_Pin_12);
#define COM2_TRANSPORT_LINE_RX              GPIO_SetBits(GPIOC, GPIO_Pin_12);
#define COM2_MULTIPROCESSOR_ENABLE          /*USART_ReceiverWakeUpCmd(UART4,ENABLE)                                  */
#define COM2_MULTIPROCESSOR_DISABLE         /*USART_ReceiverWakeUpCmd(UART4,DISABLE)                                 */

                                            /* Place your implementation of fputc here                                */
#define COM2_FPUTC(c)                       USART_SendData(UART4, (T08U) c);\
                                            while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET){}

#define COM2_RX_INTERRUPT_ENABLE            USART_ITConfig(UART4,USART_IT_RXNE,ENABLE)
#define COM2_RX_INTERRUPT_DISABLE           USART_ITConfig(UART4,USART_IT_RXNE,DISABLE)
#define COM2_RX_IRQ_FLAG                    (USART_GetITStatus(UART4,USART_IT_RXNE)!=RESET)
#define COM2_RX_BYTE_READ(result)           result=USART_ReceiveData(UART4)
#define COM2_RX_IRQ_FLAG_CLEAR              USART_ClearFlag(UART4,USART_FLAG_RXNE)

#define COM2_TIM_IRQ_FLAG_CLEAR             TIM_ClearITPendingBit(TIM5,TIM_FLAG_Update)
#define COM2_TIM_ENABLE                     TIM_Cmd(TIM5, ENABLE)
#define COM2_TIM_DISABLE                    TIM_Cmd(TIM5, DISABLE)
#define COM2_TIM_SET_COUNTER                TIM_SetCounter(TIM5, 0)

#if USE_DMA_TX_COM2
#define COM2_TX_INTERRUPT_ENABLE            USART_ITConfig(UART4,USART_IT_TC,ENABLE)
#define COM2_TX_INTERRUPT_DISABLE           USART_ITConfig(UART4,USART_IT_TC,DISABLE)
#define COM2_TX_IRQ_FLAG                    (USART_GetITStatus(UART4,USART_IT_TC)!=RESET)
#define COM2_TX_IRQ_FLAG_CLEAR              USART_ClearFlag(UART4,USART_IT_TC)
#endif
/*
************************************************************************************************************************************
*      Hardware related  functions
************************************************************************************************************************************
*/

void Com2HWPortInit(Format format, Baud baud,T32U msOfperiod);

#if USE_DMA_TX_COM2
void Com2TxMessagebyDMA(T08U *buffer,T32U size);/* DMA data sending */
#endif	/* USE_DMA_TX_COM2 */


/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif /*  USE_COM2        */
#endif /* _COM2_HW_PORT_H_ */
