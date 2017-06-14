/*
************************************************************************************************************************************
*                                    COM communication module
*
*brief       This is a COM library/Module.
*
*spercifications
*   (1)physical media   :   RS232/RS422/RS485.
*   (2)NetWork topology :   Point-to-point, Multi-dropped, Multi-point
*   (2)protocol         :   TRANSPARENT TRANSMISSION,frame/message faced,NOT byte flow faced,(like IP,UDP,NOT like TCP)
*   (3)channel          :   ten channels max.
*   (4)environment      :   with or without operating system are well surported.
*   (5)filter ability   :   strong frame header and  address filter ability,refuse message not for me silently& perfectly.
*application note
*   (1) refer to the Documents in the module pakage.
*
*                          (c)Copyright 2010-2016,Zhaiguang.
*                    Based on version 1.70 beta,developed in Shanghai#2.
*                    Based on version 1.60 beta,developed in Shanghai#2.
*                        Based on version 1.52,developed in Shanghai#2.
*                        Based on version 1.4,developed in Shanghai#1.
*                    Based on version 1.1,1.2,1.3 developed in Beijing.
*                       Based on version 1.0,developed in XinXiang.
*                                All Rights Reserved.
*
************************************************************************************************************************************
*filename     : Com.c
*brief        : This is the core internal files of COM module.
*programmer(s): ZG
*version      : V1.80
*date         : 2016-01-12
************************************************************************************************************************************
*/

#ifndef _COM_CONFIG_AND_DATA_TYPE_H_
#define _COM_CONFIG_AND_DATA_TYPE_H_

#include "BasicTypeDef.h"

#include "ComCfg.h"
/*
************************************************************************************************************************************
************************************************************************************************************************************

*                DATATYPES     of COM COMMUNICATION SERVICE MODULE 
*          follow area untill next area define globle DataTypes,NEVER modify it! 
************************************************************************************************************************************
************************************************************************************************************************************
*/

/*
************************************************************************************************************************************
*                              error return code define&meaning reference table
************************************************************************************************************************************
*/

#define COM_ERR_CODE_TIMEOUT                       0
#define COM_ERR_CODE_ARGUMENT                     -1
#define COM_ERR_CODE_EXCEPTION                    -2
#define COM_ERR_CODE_RECEIVING                    -11

/*
************************************************************************************************************************************
*                                   DATATYPES
************************************************************************************************************************************
*/


typedef enum{
#if USE_COM1
    COM1  = 0,
#endif

#if USE_COM2
    COM2  = 1,
#endif

#if USE_COM3
    COM3  = 2,
#endif

#if USE_COM4
    COM4  = 3,
#endif

#if USE_COM5
    COM5  = 4,
#endif

#if USE_COM6
    COM6  = 5,
#endif

#if USE_COM7
    COM7  = 6,
#endif

#if USE_COM8
    COM8  = 7,
#endif

#if USE_COM9
    COM9  = 8,
#endif

#if USE_COM10
COM10 = 9,
#endif

COMxx = 10                               /* Don't use this port, It's just for the enum structure define                          */
    
}COMn;


typedef enum{
    _8E1,                                /* 10bits, 1start,8data,1stop,even check                                                 */  
    _8O1,                                /* 10bits, 1start,8data,1stop,odd  check                                                 */  
    _8N2,                                /* 10bits, 1start,8data,2stop,NO   check                                                 */  

    _7E1,                                /* 10bits, 1start,7data,1stop,even check                                                 */  
    _7O1,                                /* 10bits, 1start,7data,1stop,odd  check                                                 */  
    _7N2,                                /* 10bits, 1start,7data,2stop,NO   check                                                 */  
    
    _8N1                                 /* 10bits, 1start,8data,1stop,NO check                                                   */  
}Format;                                 /* least significant bit sent first                                                      */

typedef enum{
    _1200bps    = 1200,
    _2400bps    = 2400,
    _4800bps    = 4800,
    _9600bps    = 9600,
    _19200bps   = 19200,
    _28800bps   = 28800,
    _38400bps   = 38400,
    _57600bps   = 57600,
    _115200bps  = 115200
}Baud;


typedef struct{
    COMn        ComNum;
    Format      BitperByte;
    Baud        BaudRate;

}UARTInit;


typedef enum{
    RX_IDLE,			         /* only for non-blocking mode                                                            */
    RX_RECEIVING,		         /* only for non-blocking mode                                                            */
    RX_COMPLETE,		         /* for blocking&non blocking                                                             */
    RX_TIMEOUT			         /* for blocking&non blocking                                                             */
}RxResult;                               /* tell the message receiving result,for internal use                                    */

typedef enum{
  SPECIFIC_ADDRESS,
  SPECIFIC_BROADCAST,
  ANY_ADDRESS
}AddrFilter;

typedef enum{
  TIM_WAIT_FOR_RX_TIMEOUT,
  TIM_WAIT_END_OF_FRAME
}TimMode;

typedef enum{
    COM_OPT_BLOCKING,
    COM_OPT_NON_BLOCKING
}ComOpt;

typedef struct{
    UARTInit               g_Init;

    RxResult               g_RxResult;  
    TimMode                g_timmode;
    AddrFilter             g_addrfilter;

    T08U                  *g_TxBuffer;
    T32U                   g_TxMsgSize;

    T08U                  *g_RxBuffer;
    T32U                   g_RxMsgSize;    

    T08U                   g_RxAddr;
    T32U                   g_RxBufferSize;    

    T32U                   g_TimeOut;
    T32U                   g_Counter;
    const    T08U         *g_FrameHeader;
    T08U                   g_HeaderByteNum;
    T08U                   g_NowRecvByteNum;
}ComX;

typedef struct{
    COMn          comx;
    COMn          comy;
    T08U          *RxBufferx;
    T08U          *RxBuffery;
    const T08U    *FrameHeaderx;
    const T08U    *FrameHeadery;
    T08U          HeaderByteNumx;
    T08U          HeaderByteNumy;
    T08U          RxAddrx;
    T08U          RxAddry;
    AddrFilter    rulex;
    AddrFilter    ruley;
    T32U          RxBufferSizex;
    T32U          RxBufferSizey;
    T32U          TimeOutMs;
    COMn          Channel;
}ComxyIf;

/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif
