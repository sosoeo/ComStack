/*
************************************************************************************************************************************
*                      Port/OS/environment related  files for COM communication module.
************************************************************************************************************************************
*          +-----------------------------------------------------------+
*          |                     COM  OS port for uCOSIII              |
*          +-----------------------------+---------------+-------------+

*filename     : ComOSPort.h  (where x can be 1 to 10)
*brief        :Create a semaphore for every used channel( create,post,pend) if worked in OS system.
*             :This file can be used as a template Ports file for COM service.
*programmer(s): ZG
*version      : V1.5
*date         : 2013-02-22
************************************************************************************************************************************
*/

#ifndef _COM_OS_PORT_H_
#define _COM_OS_PORT_H_

#include "ComCfg.h"

#if (COM_CFG_BLOCKING_USE_OS == 1) /* use COM module in uCOSIII environment */

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

/*                COMxx ...more Port?? It's your jobs!                                                                            */

/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/
#endif /*  (COM_CFG_BLOCKING_USE_OS == 1)        */
#endif /* _COM_OS_PORT_H_                        */
