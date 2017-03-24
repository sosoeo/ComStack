/*
************************************************************************************************************************************
*                                    COM communication module
*
*brief       This is a COM library/Module.
*
*spercifications
*   (1)physical media   :   RS232/RS422/RS485.
*   (2)NetWork topology :   Point-to-point, Multi-dropped, Multi-point
*   (2)protocol         :   TRANSPARENT TRANSMISSION
*   (3)channel          :   ten channels max.
*   (4)environment      :   with or without operating system are well surported.
*application note
*   (1) refer to the Documents in the module pakage.
*
*                          (c)Copyright 2010-2016,Zhaiguang.
*                        Based on version 1.51,developed in Shanghai.
*                        Based on version 1.4,developed in Shanghai.
*                    Based on version 1.1,1.2,1.3 developed in Beijing.
*                       Based on version 1.0,developed in XinXiang.
*                                All Rights Reserved.
*
************************************************************************************************************************************
*filename     : ComCfg.h
*brief        : user configuration of the COM communication module.
*programmer(s): ZG
*version      : V1.80
*date         : 2016-01-12
************************************************************************************************************************************
*/


#ifndef _COM_USER_CONFIGURATION_OPTIONS_H_
#define _COM_USER_CONFIGURATION_OPTIONS_H_

/*
************************************************************************************************************************************
************************************************************************************************************************************
*                           CONFIGURATION AREA
*                   of COM COMMUNICATION SERVICE MODULE 
*          follow area untill next area define configurable defines
************************************************************************************************************************************
************************************************************************************************************************************
*/

/*
************************************************************************************************************************************
*
*Description:Define the channel that will be used:
*            #define USE_COMx         y 
*Note       :1.All of the COMx port have to be well Ported before using.
*            2.x is unsigned char value(1-10). y is 1(use) or 0(not use).
*            3.you had better define the ports number that will be use in series,which
*              save some RAM memory.
*     
************************************************************************************************************************************
*/

#define USE_COM1                                              1 /* switch to 1 if  port diver of COM1   UART will be used         */
#define USE_COM2                                              1 /* switch to 1 if  port diver of COM2   UART will be used         */
#define USE_COM3                                              0 /* switch to 1 if  port diver of COM3   UART will be used         */
#define USE_COM4                                              0 /* switch to 1 if  port diver of COM4   UART will be used         */
#define USE_COM5                                              0 /* switch to 1 if  port diver of COM5   UART will be used         */
#define USE_COM6                                              0 /* switch to 1 if  port diver of COM6   UART will be used         */
#define USE_COM7                                              0 /* switch to 1 if  port diver of COM7   UART will be used         */
#define USE_COM8                                              0 /* switch to 1 if  port diver of COM8   UART will be used         */
#define USE_COM9                                              0 /* switch to 1 if  port diver of COM9   UART will be used         */
#define USE_COM10                                             0 /* switch to 1 if  port diver of COM10  UART will be used         */

#define USE_DMA_TX_COM1                                       1 /* switch to 1 if  COM1 want to send message by DMA               */
#define USE_DMA_TX_COM2                                       1 /* switch to 1 if  COM2 want to send message by DMA               */
#define USE_DMA_TX_COM3                                       1 /* switch to 1 if  COM3 want to send message by DMA               */
#define USE_DMA_TX_COM4                                       1 /* switch to 1 if  COM4 want to send message by DMA               */
#define USE_DMA_TX_COM5                                       1 /* switch to 1 if  COM5 want to send message by DMA               */
#define USE_DMA_TX_COM6                                       1 /* switch to 1 if  COM6 want to send message by DMA               */
#define USE_DMA_TX_COM7                                       1 /* switch to 1 if  COM7 want to send message by DMA               */
#define USE_DMA_TX_COM8                                       1 /* switch to 1 if  COM8 want to send message by DMA               */
#define USE_DMA_TX_COM9                                       1 /* switch to 1 if  COM9 want to send message by DMA               */
#define USE_DMA_TX_COM10                                      1 /* switch to 1 if  COM10 want to send message by DMA              */


#define COM_CFG_BLOCKING_USE_OS                               2 /* valid value 0-2,use this COM module in a operating system ??   */
                                                                /* 0 :use this COM module in a system without operating system    */
                                                                /* 1 :use this COM module in uCOSIII (version > V3.00.0)          */
                                                                /* 2 :use this COM module in uCOSII  (version >= 2.86  )          */
                                                                /* . :waiting for more operating system environment support...... */

#define COM_CFG_MULTIPLE_OF_3DOT5TIME_IN_TIMER_INT_PERIOD     2 /* timer interrupt period = this define * 3.5charTime             */
                                                                /* (must be 1-6553 at 115200bps,1-1638 at9600bps,usually use 1-4) */
                                                                /* timer interrupt period is the timebase of TIMEOUT              */ 
								/* in ComRxMessage(). default value 1 is prefered                 */
                                                                /* #WARNING: valid value 1-10,never set out of range!             */
#define BROADCAST_ADDRESS                                  0x00 /* broadcast message header                                       */

#if (USE_DMA_TX_COM1 || USE_DMA_TX_COM2 || USE_DMA_TX_COM3 || USE_DMA_TX_COM4 || USE_DMA_TX_COM5 ||\
     USE_DMA_TX_COM6 || USE_DMA_TX_COM7 || USE_DMA_TX_COM8 || USE_DMA_TX_COM9 || USE_DMA_TX_COM10)
     
#define USE_DMA_TX_COMx            1
#endif
    

/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif
