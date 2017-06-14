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
*                    Based on version 1.71 ,developed in Shanghai#2.
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

/*
************************************************************************************************************************************
*                                 INCLUDES
************************************************************************************************************************************
*/

#include"ComCfg.h"
#include"ComType.h"

#if USE_COM1
#include"Com1_Port.h"                               
#endif

#if USE_COM2
#include"Com2_Port.h"                    
#endif

#if USE_COM3
#include"Com3_Port.h"                    
#endif

#if USE_COM4
#include"Com4_Port.h"                     
#endif

#if USE_COM5
#include"Com5_Port.h"
#endif

#if USE_COM6
#include"Com6_Port.h"
#endif

#if USE_COM7
#include"Com7_Port.h"
#endif

#if USE_COM8
#include"Com8_Port.h"
#endif

#if USE_COM9
#include"Com9_Port.h"
#endif

#if USE_COM10
#include"Com10_Port.h"
#endif

#if (COM_CFG_BLOCKING_USE_OS > 0) 
#include"ComOSPort.h"
#endif

/*
************************************************************************************************************************************
*                                 GLOBAL VARIABLES
************************************************************************************************************************************
*/

#if   USE_COM10
#define TOTAL_CHANNEL_NUM    10
#elif USE_COM9
#define TOTAL_CHANNEL_NUM    9
#elif USE_COM8
#define TOTAL_CHANNEL_NUM    8
#elif USE_COM7
#define TOTAL_CHANNEL_NUM    7
#elif USE_COM6
#define TOTAL_CHANNEL_NUM    6
#elif USE_COM5
#define TOTAL_CHANNEL_NUM    5
#elif USE_COM4
#define TOTAL_CHANNEL_NUM    4
#elif USE_COM3
#define TOTAL_CHANNEL_NUM    3
#elif USE_COM2
#define TOTAL_CHANNEL_NUM    2
#elif USE_COM1
#define TOTAL_CHANNEL_NUM    1
#endif

#if TOTAL_CHANNEL_NUM > 0
static ComX  Com[TOTAL_CHANNEL_NUM];
#endif



/*
************************************************************************************************************************************
*                                  LOCAL FUNCTIONS
************************************************************************************************************************************
*/


static void ComxTxMsgStart(COMn comx,const T08U*buffer,T32U size);
static void ComxRxMsgStart(COMn comx);
static void ComxRxMsgStop (COMn comx);
static void ComxStartTimer(COMn comx);


static T08U Get3dot5CharTime(Baud b);

static void ComxRxTerminateSignal(COMn comx);
static void ComxWaitRxTerminate(COMn comx);

static void ComxMultiProcessorDisable(COMn comx);
static void ComxMultiProcessorEnable(COMn comx);

/*
************************************************************************************************************************************
*Description : initialization the UART
*Arguments   :comx            COM1,COM2,COM3,COM4...COM10.
*             format          frame format
*                             _8E1:       10bits, 1start,8data,1stop,even check  
*                             _8O1:       10bits, 1start,8data,1stop,odd  check  
*                             _8N2:       10bits, 1start,8data,2stop,NO   check  
*                             _7E1:       10bits, 1start,7data,1stop,even check  
*                             _7O1:       10bits, 1start,7data,1stop,odd  check  
*                             _7N2:       10bits, 1start,7data,2stop,NO   check  
*                             _8N1:       10bits, 1start,8data,1stop,NO   check  
*             baud            baudrate of COMx
*Returns     :1,successful.if the driver file Comx_Port.c/h is well,then here well.
*Callers     :Application
*Notes       :(1)non-reetrant function(Access common source).
*             (2)The function must do:
*                  (a)Config USART
*                    -Baudrate,
*                    -word length,
*                    -parity,
*                    -stop bits,
*                    -disable hareware flow contol
*
*                    -enable Receiver
*                    -enable Transmitter
*                    -disable Tx&Rx interrupt and other uart interrupt
*
*                (b) IF THE "Physical layer" IS RS485:
*                    -switch the GPIO to set the 485chips as RECEIVE(Rx) status.
*
*                (c).config timer:
*                    -to config a timeout at 3.5characters frames transmitting time lenth.
*                    -close the timer
*                    -disable the interrupt.
*                     (tip:the length is about 3.5*CharacterFrameBitsNumber/baudrate,
*                          wile, the "CharacterFrameBitsNumber" is related to
*                          word length config)
************************************************************************************************************************************
*/

T32S ComInit(COMn comx,Format format,Baud baud)
{
    /* timer overflow/interrupt period  */
    T32U msOfperiod = Get3dot5CharTime(baud) * COM_CFG_MULTIPLE_OF_3DOT5TIME_IN_TIMER_INT_PERIOD; /* msOfperiod must be (1 - 6553) */
   
    Com[comx].g_Init.ComNum       = comx;
    Com[comx].g_Init.BitperByte   = format;
    Com[comx].g_Init.BaudRate     = baud;
    
    Com[comx].g_RxResult          = RX_IDLE;  
    Com[comx].g_timmode           = TIM_WAIT_FOR_RX_TIMEOUT;
    Com[comx].g_addrfilter        = SPECIFIC_BROADCAST;    
      
    Com[comx].g_TxBuffer          = (T08U *)0;
    Com[comx].g_TxMsgSize         = 0;
      
    Com[comx].g_RxBuffer          = (T08U *)0;
    Com[comx].g_RxMsgSize         = 0;    
      
    Com[comx].g_RxAddr            = 0;
    Com[comx].g_RxBufferSize      = 0;
      
    Com[comx].g_TimeOut           = 0;
    Com[comx].g_Counter           = 0;
    Com[comx].g_FrameHeader       = (T08U *)0;
    Com[comx].g_HeaderByteNum     = 0;
    Com[comx].g_NowRecvByteNum    = 0;


    switch (comx) {
#if USE_COM1
        case COM1:
            Com1HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0)         
            Com1OSPortInit();
            #endif
            COM1_TRANSPORT_LINE_RX;

            return 1;
#endif
            
#if USE_COM2
        case COM2:
            Com2HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0)             
            Com2OSPortInit();            
            #endif
            COM2_TRANSPORT_LINE_RX;

            return 1;
#endif            

#if USE_COM3
        case COM3:
            Com3HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0) 
            Com3OSPortInit();            
            #endif
            COM3_TRANSPORT_LINE_RX;

            return 1;
#endif

#if USE_COM4
        case COM4:
            Com4HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0) 
            Com4OSPortInit();            
            #endif
            COM4_TRANSPORT_LINE_RX;

            return 1;
#endif
#if USE_COM5
        case COM5:
            Com5HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0)             
            Com5OSPortInit();            
            #endif         
            COM5_TRANSPORT_LINE_RX;

            return 1;
#endif

#if USE_COM6
        case COM6:
            Com6HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0)             
            Com6OSPortInit();  
            #endif
            COM6_TRANSPORT_LINE_RX;

            return 1;
#endif

#if USE_COM7
        case COM7:
            Com7HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0) 
            Com7OSPortInit(); 
            #endif
            COM7_TRANSPORT_LINE_RX;

            return 1;
#endif

#if USE_COM8
        case COM8:
            Com8HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0) 
            Com8OSPortInit(); 
            #endif
            COM8_TRANSPORT_LINE_RX;

            return 1;
#endif
#if USE_COM9
        case COM9:
            Com9HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0) 
            Com9OSPortInit();      
            #endif
            COM9_TRANSPORT_LINE_RX;

            return 1;
#endif
            
#if USE_COM10
        case COM10:
            Com10HWPortInit(format,baud,msOfperiod);
            #if (COM_CFG_BLOCKING_USE_OS > 0) 
            Com10OSPortInit(); 
            #endif
            COM10_TRANSPORT_LINE_RX;

            return 1;
#endif            
            
        default:
           return -1;
    }
}

/*
************************************************************************************************************************************
*Description :transmit a massage, check Tx status.
*Arguments   :comx             Uart port,COM1,COM2....
*              TxBuffer         Array of the transmit buffer.every bytes in the buffer untill 'TxMsgSize' will be sent 
*                               out in a continual bytes stream to Tx pin.
*             TxMsgSize     The size of the message/bytes stream.(not the size of the array) (unit: byte)

*Returns     :TxMsgSize       > 0  :A message was sent out successfully with this number bytes.
*                             = 0  :no message was sent out for a unvalid 'TxBuffer' or 'TxMsgSize'.
*Callers     :Application
*Notes       : COMx resource access is not protected,if you operate the same COMx in different task in OS,you should care it!  
************************************************************************************************************************************
*/

T32S ComTxMessage(COMn comx,const T08U * TxBuffer, T32U TxMsgSize)
{
    if(TxBuffer == (T08U *)0 || TxMsgSize < 1 ){
        return 0;                                           /* ,Tx message complete, '0' byte is sent out             */
    }
    
    ComxTxMsgStart(comx,TxBuffer,TxMsgSize);
    return TxMsgSize;      
}
/*
************************************************************************************************************************************
*Description :transmit a massage by DMA, check Tx status.
*Arguments   :comx             Uart port,COM1,COM2....
*              TxBuffer         Array of the transmit buffer.every bytes in the buffer untill 'TxMsgSize' will be sent 
*                               out in a continual bytes stream to Tx pin.
*             TxMsgSize     The size of the message/bytes stream.(not the size of the array) (unit: byte)

*Returns     :TxMsgSize       > 0  :A message was sent out successfully with this number bytes.
*                             = 0  :no message was sent out for a unvalid 'TxBuffer' or 'TxMsgSize'.
*Callers     :Application
*Notes       : COMx resource access is not protected,if you operate the same COMx in different task in OS,you should care it!  
************************************************************************************************************************************
*/
#ifdef USE_DMA_TX_COMx
T32S ComTxMessageByDMA(COMn comx,T08U * TxBuffer, T32U TxMsgSize)
{
    if(TxBuffer == (T08U *)0 || TxMsgSize < 1 ){
        return 0;                                           /* ,Tx message complete, '0' byte is sent out             */
    }
    switch(comx){
      
#if (USE_COM1 && USE_DMA_TX_COM1)
    case COM1:
	COM1_TRANSPORT_LINE_TX; 
	Com1TxMessagebyDMA(TxBuffer,TxMsgSize);
        COM1_TX_INTERRUPT_ENABLE;
	break;
#endif 

#if (USE_COM2 && USE_DMA_TX_COM2)
    case COM2:  
	COM2_TRANSPORT_LINE_TX;
	Com2TxMessagebyDMA(TxBuffer,TxMsgSize);
        COM2_TX_INTERRUPT_ENABLE;        
	break;
#endif

#if (USE_COM3 && USE_DMA_TX_COM3)
    case COM3:  
	COM3_TRANSPORT_LINE_TX;          
	Com3TxMessagebyDMA(TxBuffer,TxMsgSize);
        COM3_TX_INTERRUPT_ENABLE;        
	break;
#endif

#if (USE_COM4 && USE_DMA_TX_COM4)
    case COM4:  
	COM4_TRANSPORT_LINE_TX;
	Com4TxMessagebyDMA(TxBuffer,TxMsgSize);	    
        COM4_TX_INTERRUPT_ENABLE;
	break;
#endif

#if (USE_COM5 && USE_DMA_TX_COM5)
    case COM5:  
	COM5_TRANSPORT_LINE_TX;
	Com5TxMessagebyDMA(TxBuffer,TxMsgSize);	 
        COM5_TX_INTERRUPT_ENABLE;        
	break;
#endif

#if (USE_COM6 && USE_DMA_TX_COM6)
    case COM6:  
	COM6_TRANSPORT_LINE_TX;
	Com6TxMessagebyDMA(TxBuffer,TxMsgSize);	 
        COM6_TX_INTERRUPT_ENABLE;
	break;
#endif

#if (USE_COM7 && USE_DMA_TX_COM7)
    case COM7:  
	COM7_TRANSPORT_LINE_TX;
	Com7TxMessagebyDMA(TxBuffer,TxMsgSize);	 
        COM7_TX_INTERRUPT_ENABLE;        
	break;
#endif

#if (USE_COM8 && USE_DMA_TX_COM8)
    case COM8:  
	COM8_TRANSPORT_LINE_TX;
	Com8TxMessagebyDMA(TxBuffer,TxMsgSize);	
        COM8_TX_INTERRUPT_ENABLE;        
	break;
#endif

#if (USE_COM9 && USE_DMA_TX_COM9)
    case COM9:  
	COM9_TRANSPORT_LINE_TX;
	Com9TxMessagebyDMA(TxBuffer,TxMsgSize);	 
        COM9_TX_INTERRUPT_ENABLE;        
	break;
#endif

#if (USE_COM10 && USE_DMA_TX_COM10)
    case COM10:  
	COM10_TRANSPORT_LINE_TX;
	Com10TxMessagebyDMA(TxBuffer,TxMsgSize);
        COM10_TX_INTERRUPT_ENABLE;
	break;
#endif            
    }
    return TxMsgSize;      
}
#endif
/*
************************************************************************************************************************************
*Description :Try to receive a  message/continual bytes stream  from serial port(UART port) .
*             The message/continual bytes stream must end with idle status(more than 3.5char transmission time) on Rx pin.
*            
*Arguments   :comx            COM1,COM2,COM3,COM4...COM10.
*             RxBuffer        Array where received data place into.
*             FrameHeader     Array,if the target message(called frame) have a defined header,tell the header,else set to (T08U*)0.
*             HeaderByteNum   value,if the target message(called frame) have a defined header,tell header size,else set to (T08U)0.
*                             if valid Header is given,any without given Header frame/message will be discarded without notify.
*                             It's especially designed for some app protocol in  RS485 communication,to avoid waking up unaddressed
*                             slave device.
*             RxAddr          SlaveAddress,refer to next 'rule' item.
*             rule            SPECIFIC_ADDRESS
*                             ---> only accept message which's first byte is equal to 'RxAddr'.
*                             SPECIFIC_BROADCAST
*                             ---> only accept message which's first byte is equal to 'RxAddr' or BROADCAST_ADDRESS 
*                                  configured in ChannelConfig.h.
*                             ANY_ADDRESS
*                             ---> accept any message,which's contents no care.
*             RxBufferSize    the max size of the buffer/the 'volume' of the given buffer 'RxBuffer'.
*             TimeOutMs       Reading a message is like a job.which have a start and then a end.
*
*                             In blocking mode,ComRxMessage() start with RX_TIMEOUT and after TimeOutMs period, will 
*                             definitly end with a return RX_COMPLETE or RX_TIMEOUT status.if you specified 0 value,
*                             it will wait forever until end with a return RX_COMPLETE.
*
*                             In non-blocking mode,ComRxMessge() start with RX_IDLE and enter RX_RECEIVING before 
*                             TimeOutMs period elapse.if you execute it, you get return code indicating it's receiving
*                             status. after TimeOutMs period&no message got,if you execute it,you get a timeout status
*                             or receive complete status.if you go on execute it,you will restart a new receiving
*                             sequence.in fact,you always  execute it for it's non blocking mode.
*                             TIP: usually 0ms or 500-60000ms is prefered value.
*
*             opt             COM_OPT_BLOCKING
*                             --->if #define COM_CFG_BLOCKING_USE_OS >0:the task will suspend here,OS will schedule to do 
*                                                                      other tasks.until a message received,timeout,
*                                                                      or error happened.
*                             --->if #define COM_CFG_BLOCKING_USE_OS =0:RxMessage() will check in a 'dead' way until a 
*                                                                      message received,timeout,or error happened.
*                             COM_OPT_NON_BLOCKING
*                             ---> in forward/backward or with OS system,you execute it in a big loop. when this function
*                                  is executed first time,a message receiving start then return RX_RECEIVING,next time you
*                                  execute it you may still get RX_RECEIVING status indication. you go on invoke it in 
*                                  next loop cycle,you may get RX_COMPLETE status and return the byte number of message,
*                                  if the specified timeout elapsed and no message received,you will get a  RX_TIMEOUT
*                                  return indication.if you specified a 0 timeout,then you will ether RX_COMPLETE or 
*                                  RX_RECEIVING.
*                                  if a unusual error happened,you may get a error return indication as the same as when
*                                  blocking mode.
*                                  
*                             
*
*Returns     :RxMsgSize       > 0  :A message received with this number bytes.
*                             = 0  :no message received and timeout.only when you specified a non 0 TimeOutMs
*                             = -11:Waiting a message in non-blocking mode .in non blocking mode,the receiving have been start 
*                                   and a message is receiving or wait for  a message coming. and,timeout is not elapse.
*                             = -1 :if user give a unvlid 'RxBuffer' (null pointer)and 'RxBufferSize'(0 size,no place to store message!!).
*                             = -2 :internal error. this mean the module COM is doesn't work well for unknown ...   
*Callers     :Application
*Notes       : COMx resource access is not protected,if you operate the same COMx in different task in OS,you should care it!  
************************************************************************************************************************************
*/

T32S ComRxMessage(COMn comx,T08U *RxBuffer,const T08U *FrameHeader,T08U HeaderByteNum,T08U RxAddr,AddrFilter rule,T32U RxBufferSize,T32U TimeOutMs,ComOpt opt)
{

    if(opt == COM_OPT_BLOCKING) {

	if(RxBuffer == (T08U *)0 || RxBufferSize < (HeaderByteNum + 1) ||  HeaderByteNum > 8) {
	    return COM_ERR_CODE_ARGUMENT;                                              /* RX_MSG_PARAM_ERR                        */
	}

	if(TimeOutMs < 320 && TimeOutMs > 0 ){            /* a timout less than 320ms(not 0) is little meaningful      */
	    TimeOutMs = 320;
	}

    
	Com[comx].g_RxBuffer     = RxBuffer;  
	Com[comx].g_RxMsgSize    = 0;    
    
	Com[comx].g_RxAddr       = RxAddr;
	Com[comx].g_RxBufferSize = RxBufferSize;
    
	Com[comx].g_addrfilter   = rule;
	Com[comx].g_RxResult     = RX_RECEIVING;
    
	Com[comx].g_TimeOut      = TimeOutMs/(Get3dot5CharTime(Com[comx].g_Init.BaudRate)*COM_CFG_MULTIPLE_OF_3DOT5TIME_IN_TIMER_INT_PERIOD);
	Com[comx].g_timmode      = TIM_WAIT_FOR_RX_TIMEOUT;
	Com[comx].g_Counter      = 0;

	Com[comx].g_FrameHeader       = FrameHeader;
	Com[comx].g_HeaderByteNum     = HeaderByteNum;
	Com[comx].g_NowRecvByteNum    = 0;

	    


	ComxStartTimer(comx);                                     
    	ComxRxMsgStart(comx);

	ComxWaitRxTerminate(comx);              
#if (COM_CFG_BLOCKING_USE_OS == 0)
	while( Com[comx].g_RxResult == RX_RECEIVING ) {}      /* wait for status change by timer interrupt */
#endif

	if (Com[comx].g_RxResult == RX_COMPLETE) {
	    Com[comx].g_RxResult     = RX_IDLE;
	    if((Com[comx].g_RxBuffer == (T08U *)0) || Com[comx].g_RxMsgSize < 1 ){
		return COM_ERR_CODE_EXCEPTION;
	    }
        
	    return (T32S)Com[comx].g_RxMsgSize;             /* Rx completed     Return the size of the message ( > 0)   */
	} else {                                              /*  now  Com[comx].g_RxResult == RX_TIMEOUT               */
	    Com[comx].g_RxResult     = RX_IDLE;
	    return COM_ERR_CODE_TIMEOUT;                                         /*                  RX_MSG_TIMEOUT                   */
	}

    } else {		                                      /* opt == COM_OPT_NON_BLOCKING */

	if (Com[comx].g_RxResult == RX_RECEIVING) {

	    return COM_ERR_CODE_RECEIVING;                                       /* RxMessage() is waiting to get message */

	} else	if (Com[comx].g_RxResult == RX_IDLE) {

	    if(RxBuffer == (T08U *)0 || RxBufferSize < (HeaderByteNum + 1) ||  HeaderByteNum > 8){
		return COM_ERR_CODE_ARGUMENT;                                    /* RX_MSG_PARAM_ERR                       */
	    }

	    if(TimeOutMs < 320 && TimeOutMs > 0 ){            /* a timout less than 320ms(not 0) is little meaningful      */
		TimeOutMs = 320;
	    }

    
	    Com[comx].g_RxBuffer     = RxBuffer;  
	    Com[comx].g_RxMsgSize    = 0;    
    
	    Com[comx].g_RxAddr       = RxAddr;
	    Com[comx].g_RxBufferSize = RxBufferSize;
    
	    Com[comx].g_addrfilter   = rule;
	    Com[comx].g_RxResult     = RX_RECEIVING;
    
	    Com[comx].g_TimeOut      = TimeOutMs/(Get3dot5CharTime(Com[comx].g_Init.BaudRate)*COM_CFG_MULTIPLE_OF_3DOT5TIME_IN_TIMER_INT_PERIOD);
	    Com[comx].g_timmode      = TIM_WAIT_FOR_RX_TIMEOUT;
	    Com[comx].g_Counter      = 0;

	    Com[comx].g_FrameHeader       = FrameHeader;
	    Com[comx].g_HeaderByteNum     = HeaderByteNum;
	    Com[comx].g_NowRecvByteNum    = 0;

	    ComxStartTimer(comx);                                     
    	    ComxRxMsgStart(comx);

	    return COM_ERR_CODE_RECEIVING;                                       /* RxMessage() is waiting to get message */

	} else if (Com[comx].g_RxResult == RX_COMPLETE) {

	    Com[comx].g_RxResult     = RX_IDLE;
	    if((Com[comx].g_RxBuffer == (T08U *)0) || Com[comx].g_RxMsgSize < 1 ){
		return COM_ERR_CODE_EXCEPTION;
	    }
        
	    return (T32S)Com[comx].g_RxMsgSize;             /* Rx completed     Return the size of the message ( > 0)   */

	} else {                                              /*  now  Com[comx].g_RxResult == RX_TIMEOUT               */

	    Com[comx].g_RxResult     = RX_IDLE;
	    return COM_ERR_CODE_TIMEOUT;                                         /*                  RX_MSG_TIMEOUT           */
	}
    }
}

#if (COM_CFG_BLOCKING_USE_OS > 0)
/*
************************************************************************************************************************************
*Description :Try to receive a  message/continual bytes stream  from one of two serial ports(UART ports) .
*             The message/continual bytes stream must end with idle status(more than 3.5char transmission time) on Rx pin.
*             This function do ComRxMessage() jobs in COM_OPT_BLOCKING mode and OS support.it start two UART ports,wait for any msg
*             from the two ports.
*Arguments   :
*             If->comx
*             If->comy        
*                             COM1,COM2,COM3,COM4...COM10,comx must be different with comy.
*             If->RxBufferx   
*             If->RxBuffery   Array where received data place into,from If->comx/If->comy.
*             If->FrameHeaderx
*             If->FrameHeadery
*                             Array,if the target message(called frame) have a defined header,tell the header,else set to (T08U*)0.
*             If->HeaderByteNumx
*             If->HeaderByteNumy
*                             value,if the target message(called frame) have a defined header,tell header size,else set to (T08U)0.
*                             if valid Header is given,any without given Header frame/message will be discarded without notify.
*                             It's especially designed for some app protocol in  RS485 communication,to avoid waking up unaddressed
*                             slave device.
*             If->RxAddrx
*             If->RxAddry
*                             SlaveAddress,refer to next 'rule' item.
*             If->rulex
*             If->ruley
*                             SPECIFIC_ADDRESS
*                             ---> only accept message which's first byte is equal to 'RxAddr'.
*                             SPECIFIC_BROADCAST
*                             ---> only accept message which's first byte is equal to 'RxAddr' or BROADCAST_ADDRESS 
*                                  configured in ChannelConfig.h.
*                             ANY_ADDRESS
*                             ---> accept any message,which's contents no care.
*             If->RxBufferSizex
*             If->RxBufferSizey   
                              the max size of the buffer/the 'volume' of the given buffer 'RxBuffer'.
*             If->TimeOutMs       
*                             Reading a message is like a job.which have a start and then a end.
*
*                             In blocking mode,ComRxMessage() start with RX_TIMEOUT and after TimeOutMs period, will 
*                             definitly end with a return RX_COMPLETE or RX_TIMEOUT status.if you specified 0 value,
*                             it will wait forever until end with a return RX_COMPLETE.
*
*                             TIP: usually 0ms or 500-60000ms is prefered value.
*            If->Channel      ComXYRxMessage() will tell the application caller which COMn send the message.
*Returns     :If->RxMsgSize   > 0  :A message received with this number bytes.
*                             = 0  :no message received and timeout.only when you specified a non 0 TimeOutMs
*                             = -1 :if user give a unvlid 'RxBuffer' (null pointer)and 'RxBufferSize'(0 size,no place to store message!!).
*                             = -2 :internal error. this mean the module COM is doesn't work well for unknown ...   
*Callers     :Application
*Notes       : COMx resource access is not protected,if you operate the same COMx in different task in OS,you should care it!  
************************************************************************************************************************************
*/

T32S ComXYRxMessage(ComxyIf *If)
{
    T32U comxy;
    if(If->RxBufferx == (T08U *)0 || If->RxBufferSizex < (If->HeaderByteNumx + 1) ||  If->HeaderByteNumx > 8) {
	return COM_ERR_CODE_ARGUMENT;                                              /* RX_MSG_PARAM_ERR                        */
    }
    if(If->RxBuffery == (T08U *)0 || If->RxBufferSizey < (If->HeaderByteNumy + 1) ||  If->HeaderByteNumy > 8) {
	return COM_ERR_CODE_ARGUMENT;                                              /* RX_MSG_PARAM_ERR                        */
    }




    if(If->TimeOutMs < 320 && If->TimeOutMs > 0 ){            /* a timout less than 320ms(not 0) is little meaningful      */
	If->TimeOutMs = 320;
    }

    If->Channel                  = COMxx;

    Com[If->comx].g_RxBuffer     = If->RxBufferx;  
    Com[If->comy].g_RxBuffer     = If->RxBuffery;  
    Com[If->comx].g_RxMsgSize    = 0;    
    Com[If->comy].g_RxMsgSize    = 0;    
    
    Com[If->comx].g_RxAddr       = If->RxAddrx;
    Com[If->comy].g_RxAddr       = If->RxAddry;
    Com[If->comx].g_RxBufferSize = If->RxBufferSizex;
    Com[If->comy].g_RxBufferSize = If->RxBufferSizey;
    
    Com[If->comx].g_addrfilter   = If->rulex;
    Com[If->comy].g_addrfilter   = If->ruley;
    Com[If->comx].g_RxResult     = RX_RECEIVING;
    Com[If->comy].g_RxResult     = RX_RECEIVING;
    
    Com[If->comx].g_TimeOut      = 0;
    Com[If->comy].g_TimeOut      = 0;
    Com[If->comx].g_timmode      = TIM_WAIT_FOR_RX_TIMEOUT;
    Com[If->comy].g_timmode      = TIM_WAIT_FOR_RX_TIMEOUT;
    Com[If->comx].g_Counter      = 0;
    Com[If->comy].g_Counter      = 0;

    Com[If->comx].g_FrameHeader       = If->FrameHeaderx;
    Com[If->comy].g_FrameHeader       = If->FrameHeadery;
    Com[If->comx].g_HeaderByteNum     = If->HeaderByteNumx;
    Com[If->comy].g_HeaderByteNum     = If->HeaderByteNumy;
    Com[If->comx].g_NowRecvByteNum    = 0;
    Com[If->comy].g_NowRecvByteNum    = 0;

	    


    ComxStartTimer(If->comx);                                     
    ComxRxMsgStart(If->comx);
    ComxStartTimer(If->comy);                                     
    ComxRxMsgStart(If->comy);

    comxy = ComXYWaitRxTerminate(If->comx,If->comy,If->TimeOutMs);

    if       (comxy == (T32U)If->comx && Com[If->comx].g_RxResult == RX_COMPLETE) {
	ComxRxMsgStop(If->comx);	/* in fact comx have been stoped by timer isr */
	ComxRxMsgStop(If->comy);	/* while   If->comy is waiting for msg,we stop it */
	Com[If->comx].g_RxResult     = RX_IDLE;
	Com[If->comy].g_RxResult     = RX_IDLE;
	if((Com[If->comx].g_RxBuffer == (T08U *)0) || Com[If->comx].g_RxMsgSize < 1 ){
	    return COM_ERR_CODE_EXCEPTION;
	}
	If->Channel = If->comx;
	return (T32S)Com[If->comx].g_RxMsgSize;             /* Rx completed     Return the size of the message ( > 0)   */
    }else if (comxy == (T32U)If->comy && Com[If->comy].g_RxResult == RX_COMPLETE) {
	ComxRxMsgStop(If->comx);	/* while   comx is waiting for msg,we stop it */
	ComxRxMsgStop(If->comy);	/* in fact comy have been stoped by timer isr */
	Com[If->comx].g_RxResult     = RX_IDLE;
	Com[If->comy].g_RxResult     = RX_IDLE;
	if((Com[If->comy].g_RxBuffer == (T08U *)0) || Com[If->comy].g_RxMsgSize < 1 ){
	    return COM_ERR_CODE_EXCEPTION;
	}
	If->Channel = If->comy;
	return (T32S)Com[If->comy].g_RxMsgSize;             /* Rx completed     Return the size of the message ( > 0)   */
    } else if (comxy == 0xFF){
	ComxRxMsgStop(If->comx);	/* while   comx is waiting for msg,we stop it */
	ComxRxMsgStop(If->comy);	/* while   comy is waiting for msg,we stop it */
	Com[If->comx].g_RxResult     = RX_IDLE;
	Com[If->comy].g_RxResult     = RX_IDLE;
	return COM_ERR_CODE_TIMEOUT;                        /*Rx Receiving but timeout, Return COM_ERR_CODE_TIMEOUT(= 0)*/
    } else {
	ComxRxMsgStop(If->comx);	/* while   comx is waiting for msg,we stop it */
	ComxRxMsgStop(If->comy);	/* while   comy is waiting for msg,we stop it */
	Com[If->comx].g_RxResult     = RX_IDLE;
	Com[If->comy].g_RxResult     = RX_IDLE;
	return COM_ERR_CODE_EXCEPTION;                      /*Rx Receiving but exception, Return error,           (< 0) */
    }
}

#endif	/*  #if (COM_CFG_BLOCKING_USE_OS > 0) */

/*
************************************************************************************************************************************
*Description :get a time value with ms units. the time is equal to 3.5 bytes data transmission time in a baudrate.
*Arguments   :...
*Returns     :None.
*Callers     :internal.
*Notes       :none.
************************************************************************************************************************************
*/

static T08U Get3dot5CharTime(Baud b)
{

    if( (T32U)b >= (T32U)_38400bps) {
	return 1; 

    } else if( (T32U)b >= (T32U)_19200bps) {
	return 2; 

    } else if( (T32U)b >= (T32U)_9600bps) {
	return 4; 

    } else if( (T32U)b >= (T32U)_4800bps) {
	return 8; 

    } else if( (T32U)b >= (T32U)_2400bps) {
	return 16;

    } else if( (T32U)b >= (T32U)_1200bps) {
	return 32;
    } else {
	return 10;
    }
}




/*
************************************************************************************************************************************
*Description :Comx RX interrupt service routine, to decide whether a received data should be put into user buffer.
*Arguments   :comx  the usart port number,COM1,COM2,COM3...COM10.
*Returns     :None.
*Callers     :Comx receiver register non-empty interrupt service routine.
*Notes       :1.MUST be placed on entry of RX register non-empty ISR.
************************************************************************************************************************************
*/

void ComxRxISR(COMn comx)
{
    T08U data;
    T08U i;    
    switch(comx){
        #if USE_COM1
        case COM1:
            if(COM1_RX_IRQ_FLAG){
                COM1_RX_BYTE_READ(data);
                COM1_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif
            
        #if USE_COM2
        case COM2:
            if(COM2_RX_IRQ_FLAG){
                COM2_RX_BYTE_READ(data);
                COM2_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif            

        #if USE_COM3
        case COM3:
            if(COM3_RX_IRQ_FLAG){
                COM3_RX_BYTE_READ(data);
                COM3_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif

        #if USE_COM4
        case COM4:
            if(COM4_RX_IRQ_FLAG){
                COM4_RX_BYTE_READ(data);
                COM4_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif

        #if USE_COM5
        case COM5:
            if(COM5_RX_IRQ_FLAG){
                COM5_RX_BYTE_READ(data);
                COM5_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif

        #if USE_COM6
        case COM6:
            if(COM6_RX_IRQ_FLAG){
                COM6_RX_BYTE_READ(data);
                COM6_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif

        #if USE_COM7
        case COM7:
            if(COM7_RX_IRQ_FLAG){
                COM7_RX_BYTE_READ(data);
                COM7_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif

        #if USE_COM8
        case COM8:
            if(COM8_RX_IRQ_FLAG){
                COM8_RX_BYTE_READ(data);
                COM8_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif

        #if USE_COM9
        case COM9:
            if(COM9_RX_IRQ_FLAG){
                COM9_RX_BYTE_READ(data);
                COM9_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif

        #if USE_COM10
        case COM10:
            if(COM10_RX_IRQ_FLAG){
                COM10_RX_BYTE_READ(data);
                COM10_RX_IRQ_FLAG_CLEAR;
            } else {
                return;  /* no interrupt happened */
            }
            break;
        #endif
            
        default:
            return;        /* err */
    }
    
        

    if(Com[comx].g_FrameHeader == (T08U *)0 || Com[comx].g_HeaderByteNum == 0) {

	if (Com[comx].g_RxMsgSize == 0) {
	    if ( (Com[comx].g_RxAddr == data) || (Com[comx].g_addrfilter ==  ANY_ADDRESS) ||
		 ((data == BROADCAST_ADDRESS) && (Com[comx].g_addrfilter == SPECIFIC_BROADCAST)) ) {
                   
		ComxMultiProcessorDisable(comx);    /*                  receive the later data of this frame          */
	    } else {
		ComxMultiProcessorEnable(comx);     /* discard the later data of this frame untill next frame comming */ 
		return;
	    }
	}


	Com[comx].g_RxBuffer[Com[comx].g_RxMsgSize++] = data;
	if( Com[comx].g_RxMsgSize >= Com[comx].g_RxBufferSize){ /* rx buffer full, report receive complete  */
	    ComxRxMsgStop(comx);
	    Com[comx].g_RxResult=RX_COMPLETE;
	    ComxRxTerminateSignal(comx);
	    return;
	}
	Com[comx].g_timmode = TIM_WAIT_END_OF_FRAME;
	ComxStartTimer(comx);

    } else {
	if( Com[comx].g_RxMsgSize + 1 < Com[comx].g_HeaderByteNum){
	    Com[comx].g_RxBuffer[Com[comx].g_RxMsgSize++] = data;
	    return;
	} else if(Com[comx].g_RxMsgSize + 1 == Com[comx].g_HeaderByteNum) {
	    Com[comx].g_RxBuffer[Com[comx].g_RxMsgSize++] = data;
	    for(i = 0;i < Com[comx].g_HeaderByteNum;i++) {
		if(Com[comx].g_RxBuffer[i] != Com[comx].g_FrameHeader[i]) {
		    Com[comx].g_RxMsgSize      = 0;
		    Com[comx].g_NowRecvByteNum = 0;
		    return;
		}
	    }
	    return;
	} else if(Com[comx].g_RxMsgSize    == Com[comx].g_HeaderByteNum) {
	    if ( (Com[comx].g_RxAddr == data) || (Com[comx].g_addrfilter ==  ANY_ADDRESS) ||
		 ((data == BROADCAST_ADDRESS) && (Com[comx].g_addrfilter == SPECIFIC_BROADCAST)) ) {

		Com[comx].g_RxBuffer[Com[comx].g_RxMsgSize++] = data;
		if( Com[comx].g_RxMsgSize >= Com[comx].g_RxBufferSize){ /* rx buffer full, report receive complete  */
		    ComxRxMsgStop(comx);
		    Com[comx].g_RxResult=RX_COMPLETE;
		    ComxRxTerminateSignal(comx);
		    return;
		}
		Com[comx].g_timmode = TIM_WAIT_END_OF_FRAME;
		ComxStartTimer(comx);

	    } else {
		Com[comx].g_RxMsgSize      = 0;
		Com[comx].g_NowRecvByteNum = 0;
		return;
	    }
	/* } else if( Com[comx].g_RxMsgSize >= Com[comx].g_RxBufferSize){ */
	/*     return; */
	} else {
	    Com[comx].g_RxBuffer[Com[comx].g_RxMsgSize++] = data;
	    if( Com[comx].g_RxMsgSize >= Com[comx].g_RxBufferSize){ /* rx buffer full, report receive complete  */
		ComxRxMsgStop(comx);
		Com[comx].g_RxResult=RX_COMPLETE;
		ComxRxTerminateSignal(comx);
		return;
	    }
	    Com[comx].g_timmode = TIM_WAIT_END_OF_FRAME;
	    ComxStartTimer(comx);
	}
    }
}

/*
************************************************************************************************************************************
*Description :Comx Timer interrupt service routine, Manage message receive complete/timeout.
*Arguments   :*Arguments   :comx  the usart port number,COM1,COM2,COM3...COM10.
*Returns     :None.
*Callers     :Comx receiver register non-empty interrupt service routine.
*Notes       :1.MUST be placed on entry of Comx timer update ISR.
*
************************************************************************************************************************************
*/

void ComxTimerISR(COMn comx)
{
  switch(comx){
    
      #if USE_COM1
      case COM1:
            COM1_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM2
      case COM2:
            COM2_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM3
      case COM3:
            COM3_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM4
      case COM4:
            COM4_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM5
      case COM5:
            COM5_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM6
      case COM6:
            COM6_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM7
      case COM7:
            COM7_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM8
      case COM8:
            COM8_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM9
      case COM9:
            COM9_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      #if USE_COM10
      case COM10:
            COM10_TIM_IRQ_FLAG_CLEAR;
            break;
      #endif
            
      default: 
            return;  /* err */
  }
            

    if (Com[comx].g_timmode == TIM_WAIT_END_OF_FRAME) {

	ComxRxMsgStop(comx);

                Com[comx].g_RxResult=RX_COMPLETE;
                ComxRxTerminateSignal(comx);

        } else {/*  now         Com[comx].g_timmode==TIM_WAIT_FOR_RX_TIMEOUT */

        Com[comx].g_Counter++; 	

	if(Com[comx].g_RxMsgSize > 0) {
	    if(Com[comx].g_RxMsgSize == Com[comx].g_NowRecvByteNum) { /* FrameHeader segment is not valid because bytes timout */
		    Com[comx].g_RxMsgSize      = 0;
		    Com[comx].g_NowRecvByteNum = 0;
	    } else {
		Com[comx].g_NowRecvByteNum = Com[comx].g_RxMsgSize;
	    }
	}
	/* ifCom[comx].g_TimeOut = 0,Rxmessage() will wait forever for message.                                */
	/*(NOTE:have nothing to do with  BLOCKING(COM_OPT_BLOCKING) or NON BLOCKING(COM_OPT_NON_BLOCKING)      */
        if (Com[comx].g_Counter >= Com[comx].g_TimeOut &&  Com[comx].g_TimeOut > 0) {

	    ComxRxMsgStop(comx);


            Com[comx].g_RxResult=RX_TIMEOUT;
            ComxRxTerminateSignal(comx);

        }
    }
  /* err */
}

/*
************************************************************************************************************************************
*Description :Transmit a bytes stream by poll method.
*Arguments   :...
*Returns     :None.
*Callers     :internal.
*Notes       :none.
*
************************************************************************************************************************************
*/

static void ComxTxMsgStart(COMn comx,const T08U*buffer,T32U size)
{
    T32U TxCounter;
    switch(comx){
      
        #if USE_COM1
        case COM1:  
            COM1_TRANSPORT_LINE_TX; 
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM1_FPUTC(buffer[TxCounter]);
            }
            COM1_TRANSPORT_LINE_RX;
            break;
        #endif 

        #if USE_COM2
        case COM2:  
            COM2_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM2_FPUTC(buffer[TxCounter]);
            }
            COM2_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM3
        case COM3:  
            COM3_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM3_FPUTC(buffer[TxCounter]);
            }
            COM3_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM4
        case COM4:  
            COM4_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM4_FPUTC(buffer[TxCounter]);
            }
            COM4_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM5
        case COM5:  
            COM5_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM5_FPUTC(buffer[TxCounter]);
            }
            COM5_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM6
        case COM6:  
            COM6_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM6_FPUTC(buffer[TxCounter]);
            }
            COM6_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM7
        case COM7:  
            COM7_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM7_FPUTC(buffer[TxCounter]);
            }
            COM7_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM8
        case COM8:  
            COM8_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM8_FPUTC(buffer[TxCounter]);
            }
            COM8_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM9
        case COM9:  
            COM9_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM9_FPUTC(buffer[TxCounter]);
            }
            COM9_TRANSPORT_LINE_RX;
            break;
        #endif

        #if USE_COM10
        case COM10:  
            COM10_TRANSPORT_LINE_TX;          
            for(TxCounter = 0;TxCounter < size;TxCounter++){
                COM10_FPUTC(buffer[TxCounter]);
            }
            COM10_TRANSPORT_LINE_RX;
            break;
        #endif            
        default: return;    /* err */                          
    } 
}

/*
************************************************************************************************************************************
*Description :Be ready to receive the message.
*             Enable receive register non-empty[or FIFO full] interrupt.
*Arguments   :comx  the usart port number,COM1,COM2,COM3...COM10.
*Returns     :None.
*Callers     :internal.
*Notes       :None.
************************************************************************************************************************************
*/

static void ComxRxMsgStart(COMn comx)
{
    T08U nouse = 0x00;
    nouse = 1*nouse;    
    switch(comx){
        
        #if USE_COM1
        case COM1:
            if(COM1_RX_IRQ_FLAG){
                COM1_RX_BYTE_READ(nouse);
                COM1_RX_IRQ_FLAG_CLEAR;
            }
            COM1_RX_INTERRUPT_ENABLE;
            return;
        #endif
            
        #if USE_COM2
        case COM2:
            if(COM2_RX_IRQ_FLAG){
                COM2_RX_BYTE_READ(nouse);
                COM2_RX_IRQ_FLAG_CLEAR;
            }          
            COM2_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM3
        case COM3:
            if(COM3_RX_IRQ_FLAG){
                COM3_RX_BYTE_READ(nouse);
                COM3_RX_IRQ_FLAG_CLEAR;
            }          
            COM3_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM4
        case COM4:
            if(COM4_RX_IRQ_FLAG){
                COM4_RX_BYTE_READ(nouse);
                COM4_RX_IRQ_FLAG_CLEAR;
            }          
            COM4_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM5
        case COM5:
            if(COM5_RX_IRQ_FLAG){
                COM5_RX_BYTE_READ(nouse);
                COM5_RX_IRQ_FLAG_CLEAR;
            }          
            COM5_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM6
        case COM6:
            if(COM6_RX_IRQ_FLAG){
                COM6_RX_BYTE_READ(nouse);
                COM6_RX_IRQ_FLAG_CLEAR;
            }          
            COM6_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM7
        case COM7:
            if(COM7_RX_IRQ_FLAG){
                COM7_RX_BYTE_READ(nouse);
                COM7_RX_IRQ_FLAG_CLEAR;
            }          
            COM7_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM8
        case COM8:
            if(COM8_RX_IRQ_FLAG){
                COM8_RX_BYTE_READ(nouse);
                COM8_RX_IRQ_FLAG_CLEAR;
            }          
            COM8_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM9
        case COM9:
            if(COM9_RX_IRQ_FLAG){
                COM9_RX_BYTE_READ(nouse);
                COM9_RX_IRQ_FLAG_CLEAR;
            }          
            COM9_RX_INTERRUPT_ENABLE;
            return;
        #endif

        #if USE_COM10
        case COM10:
            if(COM10_RX_IRQ_FLAG){
                COM10_RX_BYTE_READ(nouse);
                COM10_RX_IRQ_FLAG_CLEAR;
            }          
            COM10_RX_INTERRUPT_ENABLE;
            return;
        #endif
            
        default :
            return; /* err */
    }
    

}
/*
************************************************************************************************************************************
*Description :close the serial port,stop receive the message.
*             Disable receive register non-empty[or FIFO full] interrupt.
*Arguments   :comx  the usart port number,COM1,COM2,COM3...COM10.
*Returns     :None.
*Callers     :internal.
*Notes       :None.
************************************************************************************************************************************
*/

static void ComxRxMsgStop(COMn comx)
{
    switch(comx){
                      
#if USE_COM1
    case COM1:
	COM1_RX_INTERRUPT_DISABLE;
	COM1_TIM_DISABLE;
	break;
#endif

#if USE_COM2
    case COM2:
	COM2_RX_INTERRUPT_DISABLE;
	COM2_TIM_DISABLE;
	break;
#endif

#if USE_COM3
    case COM3:
	COM3_RX_INTERRUPT_DISABLE;
	COM3_TIM_DISABLE;
	break;
#endif
                            
#if USE_COM4
    case COM4:
	COM4_RX_INTERRUPT_DISABLE;
	COM4_TIM_DISABLE;
	break;
#endif
                            
#if USE_COM5
    case COM5:
	COM5_RX_INTERRUPT_DISABLE;
	COM5_TIM_DISABLE;
	break;
#endif
                            
#if USE_COM6
    case COM6:
	COM6_RX_INTERRUPT_DISABLE;
	COM6_TIM_DISABLE;
	break;
#endif
                            
#if USE_COM7
    case COM7:
	COM7_RX_INTERRUPT_DISABLE;
	COM7_TIM_DISABLE;
	break;
#endif  
                            
#if USE_COM8
    case COM8:
	COM8_RX_INTERRUPT_DISABLE;
	COM8_TIM_DISABLE;
	break;
#endif
                            
#if USE_COM9
    case COM9:
	COM9_RX_INTERRUPT_DISABLE;
	COM9_TIM_DISABLE;
	break;
#endif
                            
#if USE_COM10
    case COM10:
	COM10_RX_INTERRUPT_DISABLE;
	COM10_TIM_DISABLE;
	break;
#endif   
                        
    default:
	return; /* err */
    }
}


/*
************************************************************************************************************************************
*Description :Enable the "get 3.5*character transmit time" timer,Set the init counter value.
*Arguments   :comx  the port number,such as COM1,COM2...
*Returns     :None.
*Callers     :internal.
*Notes       :None.
************************************************************************************************************************************
*/

static void ComxStartTimer(COMn comx)
{ 
  
    switch (comx) {
        #if USE_COM1      
        case COM1:
            COM1_TIM_IRQ_FLAG_CLEAR;          
            COM1_TIM_DISABLE;
            COM1_TIM_ENABLE;
            COM1_TIM_SET_COUNTER;
            return;
        #endif
            
        #if USE_COM2      
        case COM2:
            COM2_TIM_IRQ_FLAG_CLEAR;          
            COM2_TIM_DISABLE;
            COM2_TIM_ENABLE;
            COM2_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM3      
        case COM3:
            COM3_TIM_IRQ_FLAG_CLEAR;          
            COM3_TIM_DISABLE;
            COM3_TIM_ENABLE;
            COM3_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM4      
        case COM4:
            COM4_TIM_IRQ_FLAG_CLEAR;          
            COM4_TIM_DISABLE;
            COM4_TIM_ENABLE;
            COM4_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM5      
        case COM5:
            COM5_TIM_IRQ_FLAG_CLEAR;          
            COM5_TIM_DISABLE;
            COM5_TIM_ENABLE;
            COM5_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM6      
        case COM6:
            COM6_TIM_IRQ_FLAG_CLEAR;          
            COM6_TIM_DISABLE;
            COM6_TIM_ENABLE;
            COM6_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM7      
        case COM7:
            COM7_TIM_IRQ_FLAG_CLEAR;          
            COM7_TIM_DISABLE;
            COM7_TIM_ENABLE;
            COM7_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM8      
        case COM8:
            COM8_TIM_IRQ_FLAG_CLEAR;          
            COM8_TIM_DISABLE;
            COM8_TIM_ENABLE;
            COM8_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM9      
        case COM9:
            COM9_TIM_IRQ_FLAG_CLEAR;          
            COM9_TIM_DISABLE;
            COM9_TIM_ENABLE;
            COM9_TIM_SET_COUNTER;
            return;
        #endif

        #if USE_COM10      
        case COM10:
            COM10_TIM_IRQ_FLAG_CLEAR;          
            COM10_TIM_DISABLE;
            COM10_TIM_ENABLE;
            COM10_TIM_SET_COUNTER;
            return;
        #endif
        default:
            return;/* err */
    }       
}

/*
************************************************************************************************************************************
*Description :Post a signal to tell the system that receiver was terminated.(whitch means
*             a message was received,or timeout happened,or err happened.)
*Arguments   :comx  the port number,such as COM1,COM2...
*Returns     :None.
*Callers     :internal.
*Notes       :1 not used in non-blocking mode.may be a warning come out when compiling.
*             2.not used in blocking mode but without OS environment.
************************************************************************************************************************************
*/

static void ComxRxTerminateSignal(COMn comx)
{
#if (COM_CFG_BLOCKING_USE_OS > 0) /* use COM module in OS environment */  
    switch(comx){
        #if USE_COM1  
        case COM1:
            Com1RxTerminateSignal(); 
            return;
        #endif
          
        #if USE_COM2  
        case COM2:
            Com2RxTerminateSignal(); 
            return;
        #endif
            
        #if USE_COM3  
        case COM3:
            Com3RxTerminateSignal(); 
            return;
        #endif

        #if USE_COM4  
        case COM4:
            Com4RxTerminateSignal(); 
            return;
        #endif

        #if USE_COM5  
        case COM5:
            Com5RxTerminateSignal(); 
            return;
        #endif

        #if USE_COM6  
        case COM6:
            Com6RxTerminateSignal(); 
            return;
        #endif

        #if USE_COM7  
        case COM7:
            Com7RxTerminateSignal(); 
            return;
        #endif

        #if USE_COM8  
        case COM8:
            Com8RxTerminateSignal(); 
            return;
        #endif

        #if USE_COM9  
        case COM9:
            Com9RxTerminateSignal(); 
            return;
        #endif

        #if USE_COM10  
        case COM10:
            Com10RxTerminateSignal(); 
            return;
        #endif
          
        default:
            return; /*err */
    }
#endif    /* COM_CFG_BLOCKING_USE_OS > 0 */
}

/*
************************************************************************************************************************************
*Description :Pend a signal which tell the system that receiver was terminated.(whitch means
*             a message was received,or timeout happened,or err happened.)
*Arguments   :comx  the port number,such as COM1,COM2...
*Returns     :None.
*Callers     :internal.
*Notes       :1 not used in non-blocking mode.may be a warning come out when compiling.
*             2.not used in blocking mode but without OS environment.
************************************************************************************************************************************
*/

static void ComxWaitRxTerminate(COMn comx)
{
#if (COM_CFG_BLOCKING_USE_OS > 0) /* use COM module in OS environment */    
    switch(comx){
        #if USE_COM1  
        case COM1:
            Com1WaitRxTerminate(); 
            return;
        #endif
          
        #if USE_COM2  
        case COM2:
            Com2WaitRxTerminate(); 
            return;
        #endif
            
        #if USE_COM3  
        case COM3:
            Com3WaitRxTerminate(); 
            return;
        #endif

        #if USE_COM4  
        case COM4:
            Com4WaitRxTerminate(); 
            return;
        #endif

        #if USE_COM5  
        case COM5:
            Com5WaitRxTerminate(); 
            return;
        #endif

        #if USE_COM6  
        case COM6:
            Com6WaitRxTerminate(); 
            return;
        #endif

        #if USE_COM7  
        case COM7:
            Com7WaitRxTerminate(); 
            return;
        #endif

        #if USE_COM8  
        case COM8:
            Com8WaitRxTerminate(); 
            return;
        #endif

        #if USE_COM9  
        case COM9:
            Com9WaitRxTerminate(); 
            return;
        #endif

        #if USE_COM10  
        case COM10:
            Com10WaitRxTerminate(); 
            return;
        #endif
          
        default:
            return; /*err */
    }
#endif    /* COM_CFG_BLOCKING_USE_OS > 0 */
}

/*
************************************************************************************************************************************
*Description :Disable  MULTIPROCESSOR COMMUNICATION MODE control bit in MCU.
*Arguments   :comx  the port number,such as COM1,COM2...
*Returns     :None.
*Callers     :internal.
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
*/
static void ComxMultiProcessorDisable(COMn comx)
{
    switch(comx){
        #if USE_COM1  
        case COM1:
            COM1_MULTIPROCESSOR_DISABLE; 
            return;
        #endif
          
        #if USE_COM2  
        case COM2:
            COM2_MULTIPROCESSOR_DISABLE; 
            return;
        #endif
            
        #if USE_COM3  
        case COM3:
            COM3_MULTIPROCESSOR_DISABLE; 
            return;
        #endif

        #if USE_COM4  
        case COM4:
            COM4_MULTIPROCESSOR_DISABLE; 
            return;
        #endif

        #if USE_COM5  
        case COM5:
            COM5_MULTIPROCESSOR_DISABLE; 
            return;
        #endif

        #if USE_COM6  
        case COM6:
            COM6_MULTIPROCESSOR_DISABLE; 
            return;
        #endif

        #if USE_COM7  
        case COM7:
            COM7_MULTIPROCESSOR_DISABLE; 
            return;
        #endif

        #if USE_COM8  
        case COM8:
            COM8_MULTIPROCESSOR_DISABLE; 
            return;
        #endif

        #if USE_COM9  
        case COM9:
            COM9_MULTIPROCESSOR_DISABLE; 
            return;
        #endif

        #if USE_COM10  
        case COM10:
            COM10_MULTIPROCESSOR_DISABLE; 
            return;
        #endif
          
        default:
            return; /*err */
    }
}
/*
************************************************************************************************************************************
*Description :Enable  MULTIPROCESSOR COMMUNICATION MODE control bit in MCU.
*Arguments   :comx  the port number,such as COM1,COM2...
*Returns     :None.
*Callers     :internal functions...
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
*/
static void ComxMultiProcessorEnable(COMn comx)
{
    switch(comx){
        #if USE_COM1  
        case COM1:
            COM1_MULTIPROCESSOR_ENABLE; 
            return;
        #endif
          
        #if USE_COM2  
        case COM2:
            COM2_MULTIPROCESSOR_ENABLE; 
            return;
        #endif
            
        #if USE_COM3  
        case COM3:
            COM3_MULTIPROCESSOR_ENABLE;
            return;
        #endif

        #if USE_COM4  
        case COM4:
            COM4_MULTIPROCESSOR_ENABLE; 
            return;
        #endif

        #if USE_COM5  
        case COM5:
            COM5_MULTIPROCESSOR_ENABLE;
            return;
        #endif

        #if USE_COM6  
        case COM6:
            COM6_MULTIPROCESSOR_ENABLE;
            return;
        #endif

        #if USE_COM7  
        case COM7:
            COM7_MULTIPROCESSOR_ENABLE;
            return;
        #endif

        #if USE_COM8  
        case COM8:
            COM8_MULTIPROCESSOR_ENABLE; 
            return;
        #endif

        #if USE_COM9  
        case COM9:
            COM9_MULTIPROCESSOR_ENABLE;
            return;
        #endif

        #if USE_COM10  
        case COM10:
            COM10_MULTIPROCESSOR_ENABLE;
            return;
        #endif
          
        default:
            return; /*err */
    }
}
/*
************************************************************************************************************************************
*                                  file end
************************************************************************************************************************************
*/









