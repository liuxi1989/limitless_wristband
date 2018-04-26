/*-----------------------------------------------------------------------------
 *      Name:         cmsis_rv.h 
 *      Purpose:      cmsis_rv header
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#ifndef __CMSIS_RV_H
#define __CMSIS_RV_H

#include <stdint.h>
#include "nrf.h"

/* Expansion macro used to create CMSIS Driver references */
#define EXPAND_SYMBOL(name, port) name##port
#define CREATE_SYMBOL(name, port) EXPAND_SYMBOL(name, port)

// Interrupt handling
extern void (*TST_IRQHandler)(void);
extern uint32_t ISR_ExNum;

// Test main function
extern void cmsis_rv (void);

// Test cases
extern void TC_ThreadCreate (void);
extern void TC_ThreadMultiInstance (void);
extern void TC_ThreadTerminate (void);
extern void TC_ThreadRestart (void);
extern void TC_ThreadGetId (void);
extern void TC_ThreadPriority (void);
extern void TC_ThreadPriorityExec (void);
extern void TC_ThreadChainedCreate (void);
extern void TC_ThreadYield (void);
extern void TC_ThreadParam (void);
extern void TC_ThreadInterrupts (void);

extern void TC_GenWaitBasic (void);
extern void TC_GenWaitInterrupts (void);

extern void TC_TimerOneShot (void);
extern void TC_TimerPeriodic (void);
extern void TC_TimerParam (void);
extern void TC_TimerInterrupts (void);

extern void TC_SignalMainThread (void);  
extern void TC_SignalChildThread (void); 
extern void TC_SignalChildToParent (void);
extern void TC_SignalChildToChild (void); 
extern void TC_SignalWaitTimeout (void); 
extern void TC_SignalParam (void);       
extern void TC_SignalInterrupts (void);  

extern void TC_MutexBasic (void);
extern void TC_MutexTimeout (void); 
extern void TC_MutexNestedAcquire (void); 
extern void TC_MutexPriorityInversion (void); 
extern void TC_MutexOwnership (void); 
extern void TC_MutexParam (void); 
extern void TC_MutexInterrupts (void); 

extern void TC_SemaphoreCreateAndDelete (void);
extern void TC_SemaphoreObtainCounting (void);
extern void TC_SemaphoreObtainBinary (void);
extern void TC_SemaphoreWaitForBinary (void);
extern void TC_SemaphoreWaitForCounting (void);
extern void TC_SemaphoreZeroCount (void);
extern void TC_SemaphoreWaitTimeout (void);
extern void TC_SemParam (void);
extern void TC_SemInterrupts (void);

extern void CreateMemoryPool (void);
extern void TC_MemPoolAllocAndFree (void);
extern void TC_MemPoolAllocAndFreeComb (void);
extern void TC_MemPoolZeroInit (void);
extern void TC_MemPoolParam (void);
extern void TC_MemPoolInterrupts (void);

extern void CreateMessageQueue (void);
extern void TC_MsgQBasic (void);
extern void TC_MsgQWait (void);
extern void TC_MsgQParam (void);
extern void TC_MsgQInterrupts (void);
extern void TC_MsgFromThreadToISR (void);
extern void TC_MsgFromISRToThread (void);

extern void CreateMailQueue (void);
extern void TC_MailAlloc (void);
extern void TC_MailCAlloc (void);
extern void TC_MailToThread (void);
extern void TC_MailFromThread (void);
extern void TC_MailTimeout (void);
extern void TC_MailParam (void);
extern void TC_MailInterrupts (void);
extern void TC_MailFromThreadToISR (void);
extern void TC_MailFromISRToThread (void);

extern void StartCortexCycleCounter (void);
extern void TC_MeasureOsDelayTicks (void);
extern void TC_MeasureOsWaitTicks (void);
extern void TC_MeasureOsSignalWaitTicks (void);
extern void TC_MeasureOsMutexWaitTicks (void);
extern void TC_MeasureOsSemaphoreWaitTicks (void);
extern void TC_MeasureOsMessageWaitTicks (void);
extern void TC_MeasureOsMailWaitTicks (void);

#endif /* __CMSIS_RV_H */
