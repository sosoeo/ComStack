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
*                    Based on version 1.71,developed in Shanghai#2.
*                    Based on version 1.70 beta,developed in Shanghai#2.
*                    Based on version 1.60 beta,developed in Shanghai#2.
*                        Based on version 1.52,developed in Shanghai#2.
*                        Based on version 1.4,developed in Shanghai#1.
*                    Based on version 1.1,1.2,1.3 developed in Beijing.
*                       Based on version 1.0,developed in XinXiang.
*                                All Rights Reserved.
************************************************************************************************************************************
*filename     : Com.h
*brief        : This is the core internal files of COM module.
*programmer(s): ZG
*version      : V1.80
*date         : 2016-01-12
************************************************************************************************************************************
*/

/*
************************************************************************************************************************************
*                                 INCLUDES
************************************************************************************************************************************
*/

#ifndef _COM_TRANSPARENT_TRANSMISSION_SERVICE_H_
#define _COM_TRANSPARENT_TRANSMISSION_SERVICE_H_

#include "ComCfg.h"
#include "ComType.h"

/*
************************************************************************************************************************************
*                                       COM communication service 
*                                       User application  layer
************************************************************************************************************************************
*/

T32S ComInit(COMn comx,Format format,Baud baud);
T32S ComTxMessage(COMn comx,const T08U * TxBuffer, T32U TxMsgSize);
T32S ComRxMessage(COMn comx,T08U *RxBuffer,const T08U *FrameHeader,T08U HeaderByteNum,
                  T08U RxAddr,AddrFilter rule,T32U RxBufferSize,T32U TimeOutMs,ComOpt opt);
T32S ComXYRxMessage(ComxyIf *If);

/*
************************************************************************************************************************************
*                                 COM communication service 
*                                  Service interface  layer
************************************************************************************************************************************
*/
void ComxRxISR(COMn comx);

void ComxTimerISR(COMn comx);
/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif
