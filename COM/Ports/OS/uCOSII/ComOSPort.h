/*
************************************************************************************************************************************
*                      Port/OS/environment related  files for COM communication module.
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     COM  OS port for uCOSII(>=v2.86)      |
*          +-----------------------------+---------------+-------------+

*filename     : ComOSPort.h  (where x can be 1 to 10)
*brief        :Create a semaphore for every used channel( create,post,pend) if worked in OS system.
*             :This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.70 beta
*date         : 2014-09-04
************************************************************************************************************************************
*/

#ifndef _COM_OS_PORT_H_
#define _COM_OS_PORT_H_

#include "ComCfg.h"

#if (COM_CFG_BLOCKING_USE_OS == 2) /* use COM module in uCOSII(>=v2.86)  environment                                              */

/*
************************************************************************************************************************************
************************************************************************************************************************************
*      OS port related declare
************************************************************************************************************************************
************************************************************************************************************************************
*/


#if USE_COM1
void Com1OSPortInit          (void);
void Com1RxTerminateSignal   (void);
void Com1WaitRxTerminate     (void); 
#endif

#if USE_COM2
void Com2OSPortInit          (void);
void Com2RxTerminateSignal   (void);
void Com2WaitRxTerminate     (void); 
#endif

#if USE_COM3
void Com3OSPortInit         (void);
void Com3RxTerminateSignal  (void);
void Com3WaitRxTerminate    (void); 
#endif 

#if USE_COM4
void Com4OSPortInit         (void);
void Com4RxTerminateSignal  (void);
void Com4WaitRxTerminate    (void); 
#endif

#if USE_COM5
void Com5OSPortInit         (void);
void Com5RxTerminateSignal  (void);
void Com5WaitRxTerminate    (void); 
#endif

#if USE_COM6
void Com6OSPortInit         (void);
void Com6RxTerminateSignal  (void);
void Com6WaitRxTerminate    (void); 
#endif

#if USE_COM7
void Com7OSPortInit         (void);
void Com7RxTerminateSignal  (void);
void Com7WaitRxTerminate    (void); 
#endif

#if USE_COM8
void Com8OSPortInit         (void);
void Com8RxTerminateSignal  (void);
void Com8WaitRxTerminate    (void); 
#endif

#if USE_COM9
void Com9OSPortInit         (void);
void Com9RxTerminateSignal  (void);
void Com9WaitRxTerminate    (void); 
#endif

#if USE_COM10
void Com10OSPortInit         (void);
void Com10RxTerminateSignal  (void);
void Com10WaitRxTerminate    (void); 
#endif

/*                COMxx ...more Port?? It's your jobs!                                                                            */


T32U ComXYWaitRxTerminate(COMn comx,COMn comy,T16U timeout);

/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif /* use COM module in uCOSII(>=v2.86)  environment                                                                          */
#endif /* _COM_OS_PORT_H_                                                                                                         */
