/*-----------------------------------------------------------------------------
 *      Name:         TC_WaitFunc.c 
 *      Purpose:      CMSIS RTOS Wait Functions Test
 *-----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include <string.h>
#include "RV_Typedefs.h"
#include "cmsis_rv.h"
#include "cmsis_os.h"
#include "nrf_drv_timer.h"

/*-----------------------------------------------------------------------------
 *      Test configuration
 *----------------------------------------------------------------------------*/
#define RTOS_TICK_TIME          1007   /* Tick time in us                    */

#ifdef SOFTDEVICE_PRESENT
#define ACCURACY        3100    // up to 3 ticks taken by sd handler
#else
#define ACCURACY        50
#endif

#define ACCURACY_OS_DELAY           ACCURACY   /* Wait accuracy in promiles          */
#define ACCURACY_OS_WAIT            ACCURACY   /* Wait accuracy in promiles          */
#define ACCURACY_OS_SIGNAL_WAIT     ACCURACY   /* Wait accuracy in promiles          */
#define ACCURACY_OS_MUTEX_WAIT      ACCURACY   /* Wait accuracy in promiles          */
#define ACCURACY_OS_SEMAPHORE_WAIT  ACCURACY   /* Wait accuracy in promiles          */
#define ACCURACY_OS_MESSAGE_WAIT    ACCURACY   /* Wait accuracy in promiles          */
#define ACCURACY_OS_MAIL_WAIT       ACCURACY   /* Wait accuracy in promiles          */

/*-----------------------------------------------------------------------------
 *      USER DEFINES
 *----------------------------------------------------------------------------*/

static inline uint16_t sub(uint16_t val1, uint16_t val2)
{
    return val1 - val2;
}


/*-----------------------------------------------------------------------------
 *      Test implementation
 *----------------------------------------------------------------------------*/
#define CPU_CYC(us) (((uint64_t)(us) * 250000) / 1000000)
uint64_t TickCyc;

uint32_t Lim_OsDelay[2];
uint32_t Lim_OsWait[2];
uint32_t Lim_OsSignalWait[2];
uint32_t Lim_OsMutexWait[2];
uint32_t Lim_OsSemaphoreWait[2];
uint32_t Lim_OsMessageWait[2];
uint32_t Lim_OsMailWait[2];

extern const nrf_drv_timer_t TIMER_TICKS;
/* Definitions for accessing Cortex registers */
#define GET_TICKS          nrf_drv_timer_capture(&TIMER_TICKS, NRF_TIMER_CC_CHANNEL0)

static uint8_t CycCntRunning;

/* Definitions of shared variables */
osMutexId       G_WaitFunc_MutexId;              /* Global mutex id                    */
osThreadId      G_WaitFunc_ThreadId;             /* Global thread id                   */
osSemaphoreId   G_WaitFunc_SemaphoreId;          /* Global semaphore id                */
osMessageQId    G_WaitFunc_MessageQId;           /* Global message queue id            */
osMailQId       G_WaitFunc_MailQId;              /* Global mail queue id               */

/* Definitions for TC_MeasureOsMutexWaitTicks */
static void  WaitFunc_Th_MutexLock (void const *arg);
osThreadDef (WaitFunc_Th_MutexLock, osPriorityAboveNormal, 1, 0);

osMutexDef (WaitFunc_MutexTout);

/* Definitions for TC_MeasureOsSemaphoreWaitTicks */
osSemaphoreDef (SemaphoreTout);

/* Definitions for TC_MeasureOsMessageWaitTicks */
osMessageQDef (MessageQTout, 1, uint32_t);

/* Definitions for TC_MeasureOsMailWaitTicks */
typedef struct {
  uint8_t Buf[4];
} MAIL_OBJ;
osMailQDef (MailQTout, 1, MAIL_OBJ);

MAIL_OBJ *G_MailPtr;

/*-----------------------------------------------------------------------------
 *      Initialization: ensure that cycle counter is running
 *----------------------------------------------------------------------------*/
void StartCortexCycleCounter (void) {
  uint32_t t[2];
  
  /* If cycle counter not supported then this test won't be executed */
  CycCntRunning = 0;

  osDelay(5);

  t[0] = GET_TICKS;

  osDelay(1);

  t[1] = GET_TICKS;

  ASSERT_TRUE(t[1] - t[0]);
    
  /* Ensure proper clocking */
  SystemCoreClockUpdate ();
  
  /* Calculate number of cycles within one system tick */
  TickCyc = CPU_CYC (RTOS_TICK_TIME);

  /* Calculate accuracy limits */
  Lim_OsDelay[0]         = TickCyc + (TickCyc * ACCURACY_OS_DELAY / 1000);
  Lim_OsDelay[1]         = TickCyc - (TickCyc * ACCURACY_OS_DELAY / 1000);

  Lim_OsWait[0]          = TickCyc + (TickCyc * ACCURACY_OS_WAIT / 1000);
  Lim_OsWait[1]          = TickCyc - (TickCyc * ACCURACY_OS_WAIT / 1000);

  Lim_OsSignalWait[0]    = TickCyc + (TickCyc * ACCURACY_OS_SIGNAL_WAIT / 1000);
  Lim_OsSignalWait[1]    = TickCyc - (TickCyc * ACCURACY_OS_SIGNAL_WAIT / 1000);

  Lim_OsMutexWait[0]     = TickCyc + (TickCyc * ACCURACY_OS_MUTEX_WAIT / 1000);
  Lim_OsMutexWait[1]     = TickCyc - (TickCyc * ACCURACY_OS_MUTEX_WAIT / 1000);

  Lim_OsSemaphoreWait[0] = TickCyc + (TickCyc * ACCURACY_OS_SEMAPHORE_WAIT / 1000);
  Lim_OsSemaphoreWait[1] = TickCyc - (TickCyc * ACCURACY_OS_SEMAPHORE_WAIT / 1000);

  Lim_OsMessageWait[0]   = TickCyc + (TickCyc * ACCURACY_OS_MESSAGE_WAIT / 1000);
  Lim_OsMessageWait[1]   = TickCyc - (TickCyc * ACCURACY_OS_MESSAGE_WAIT / 1000);

  Lim_OsMailWait[0]      = TickCyc + (TickCyc * ACCURACY_OS_MAIL_WAIT / 1000);
  Lim_OsMailWait[1]      = TickCyc - (TickCyc * ACCURACY_OS_MAIL_WAIT / 1000);
}


/*-----------------------------------------------------------------------------
 * Mutex locking thread
 *----------------------------------------------------------------------------*/
static void WaitFunc_Th_MutexLock (void const *arg) {
  /* Obtain a mutex */
  ASSERT_TRUE (osMutexWait (G_WaitFunc_MutexId, 0) == osOK);

  osDelay (1000);                       /* This call should never return      */
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup waitfunc_funcs Wait Functions
\brief Wait Functions Test Cases
\details
The test cases measure the ticks of all wait/delay related functions (such as osDelay, osWait, osSempahoreWait, etc.).

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MeasureOsDelayTicks
\details
- Measure approximate tick length during osDelay
- Check if cycle count is within defined accuracy
*/
void TC_MeasureOsDelayTicks (void) {
  osThreadId id;
  uint16_t m0[2], m1[2];
  uint32_t dif, approx_tick;

  if (CycCntRunning) {
    /* Raise priority of control thread to realtime */
    id = osThreadGetId();
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityRealtime) == osOK);
  
    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osDelay (100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osDelay (101);
    m1[1] = GET_TICKS;

    approx_tick = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osDelay(1);
    m0[1] = GET_TICKS;

    dif = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (approx_tick > Lim_OsDelay[0] && approx_tick < Lim_OsDelay[1]);
    ASSERT_TRUE (dif > Lim_OsDelay[0] && dif < Lim_OsDelay[1]);

    /* Change priority back to normal */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MeasureOsWaitTicks
\details
- Measure approximate tick length during osWait
- Check if cycle count is within defined accuracy
*/
void TC_MeasureOsWaitTicks (void) {
#if (osFeature_Wait)
  osThreadId id;
  uint16_t m0[2], m1[2];
  uint32_t wait, long_wait;

  if (CycCntRunning) {
    /* Raise priority of this thread to realtime */
    id = osThreadGetId();
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityRealtime) == osOK);
  
    /* Measure approximate tick length */

    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osWait (100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osWait (101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osWait (1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsWait[0] && long_wait < Lim_OsWait[1]);
    ASSERT_TRUE (wait > Lim_OsWait[0] && wait < Lim_OsWait[1]);
    
    /* Change priority back to normal */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
  }
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MeasureOsSignalWaitTicks
\details
- Measure approximate tick length during osSignalWait
- Check if cycle count is within defined accuracy
*/
void TC_MeasureOsSignalWaitTicks (void) {
#if (osFeature_Signals)
  osThreadId id;
  uint16_t m0[2], m1[2];
  uint32_t wait, long_wait;

  if (CycCntRunning) {
    /* Raise priority of this thread to realtime */
    id = osThreadGetId();
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityRealtime) == osOK);
  
    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osSignalWait (0x01, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osSignalWait (0x01, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osSignalWait (0x01, 1);
    m0[1] = GET_TICKS;
    
    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsSignalWait[0] && long_wait < Lim_OsSignalWait[1]);
    ASSERT_TRUE (wait > Lim_OsSignalWait[0] && wait < Lim_OsSignalWait[1]);
    
    /* Change priority back to normal */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
  }
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MeasureOsMutexWaitTicks
\details
- Measure approximate tick length during osMutexWait
- Check if cycle count is within defined accuracy
*/
void TC_MeasureOsMutexWaitTicks (void) {
  osThreadId id;
  uint16_t m0[2], m1[2];
  uint32_t wait, long_wait;

  if (CycCntRunning) {
    /* Ensure main thread priority is normal */
    id = osThreadGetId();
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
    
    /* Create a mutex object */
    G_WaitFunc_MutexId = osMutexCreate (osMutex (WaitFunc_MutexTout));
    ASSERT_TRUE (G_WaitFunc_MutexId != NULL);
    
    /* Create a thread that locks mutex */
    G_WaitFunc_ThreadId = osThreadCreate (osThread (WaitFunc_Th_MutexLock), NULL);
    ASSERT_TRUE (G_WaitFunc_ThreadId != NULL);
    
    /* Raise priority of this thread to realtime */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityRealtime) == osOK);
  
    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osMutexWait (G_WaitFunc_MutexId, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osMutexWait (G_WaitFunc_MutexId, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osMutexWait (G_WaitFunc_MutexId, 1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsMutexWait[0] && long_wait < Lim_OsMutexWait[1]);
    ASSERT_TRUE (wait > Lim_OsMutexWait[0] && wait < Lim_OsMutexWait[1]);

    /* Delete mutex and terminate locking thread */
    ASSERT_TRUE (osMutexDelete (G_WaitFunc_MutexId) == osOK);
    ASSERT_TRUE (osThreadTerminate (G_WaitFunc_ThreadId) == osOK);

    /* Change priority back to normal */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MeasureOsSemaphoreWaitTicks
\details
- Measure approximate tick length during osSemaphoreWait
- Check if cycle count is within defined accuracy
*/
void TC_MeasureOsSemaphoreWaitTicks (void) {
  osThreadId id;
  uint16_t m0[2], m1[2];
  uint32_t wait, long_wait;

  if (CycCntRunning) {
    /* Ensure main thread priority is normal */
    id = osThreadGetId();
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);

    /* Create a semaphore object */
    G_WaitFunc_SemaphoreId = osSemaphoreCreate (osSemaphore (SemaphoreTout), 0);
    ASSERT_TRUE (G_WaitFunc_SemaphoreId != NULL);

    /* Raise priority of this thread to realtime */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityRealtime) == osOK);

    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osSemaphoreWait (G_WaitFunc_SemaphoreId, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osSemaphoreWait (G_WaitFunc_SemaphoreId, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osSemaphoreWait (G_WaitFunc_SemaphoreId, 1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsSemaphoreWait[0] && long_wait < Lim_OsSemaphoreWait[1]);
    ASSERT_TRUE (wait > Lim_OsSemaphoreWait[0] && wait < Lim_OsSemaphoreWait[1]);

    /* Delete a semaphore object */
    ASSERT_TRUE (osSemaphoreDelete (G_WaitFunc_SemaphoreId) == osOK);

    /* Change priority back to normal */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MeasureOsMessageWaitTicks
\details
- Measure approximate tick length during osMessageGet and osMessagePut
- Check if cycle count is within defined accuracy
*/
void TC_MeasureOsMessageWaitTicks (void) {
#if (osFeature_MessageQ)
  osThreadId id;
  uint16_t m0[2], m1[2];
  uint32_t wait, long_wait;

  if (CycCntRunning) {
    /* Ensure main thread priority is normal */
    id = osThreadGetId();
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);

    /* Create a message queue object */
    G_WaitFunc_MessageQId = osMessageCreate (osMessageQ (MessageQTout), NULL);
    ASSERT_TRUE (G_WaitFunc_MessageQId != NULL);

    /* Raise priority of this thread to realtime */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityRealtime) == osOK);
    
    
    /* Measure osMessageGet */

    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osMessageGet (G_WaitFunc_MessageQId, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osMessageGet (G_WaitFunc_MessageQId, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osMessageGet (G_WaitFunc_MessageQId, 1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsMessageWait[0] && long_wait < Lim_OsMessageWait[1]);
    ASSERT_TRUE (wait > Lim_OsMessageWait[0] && wait < Lim_OsMessageWait[1]);
    
    /* Fill message queue */
    ASSERT_TRUE (osMessagePut (G_WaitFunc_MessageQId, 0xAA55AA55, 0) == osOK);

    
    /* Measure osMessagePut */

    /* Reset timer. */
    nrf_drv_timer_clear(&TIMER_TICKS);

    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osMessagePut (G_WaitFunc_MessageQId, 0x55AA55AA, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osMessagePut (G_WaitFunc_MessageQId, 0x55AA55AA, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osMessagePut (G_WaitFunc_MessageQId, 0x55AA55AA, 1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsMessageWait[0] && long_wait < Lim_OsMessageWait[1]);
    ASSERT_TRUE (wait > Lim_OsMessageWait[0] && wait < Lim_OsMessageWait[1]);

    /* Change priority back to normal */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
  }
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MeasureOsMailWaitTicks
\details
- Measure approximate tick length during osMailGet and osMailCAlloc
- Check if cycle count is within defined accuracy
*/
void TC_MeasureOsMailWaitTicks (void) {
#if (osFeature_MailQ)
  osThreadId id;
  uint16_t m0[2], m1[2];
  uint32_t wait, long_wait;

  if (CycCntRunning) {
    /* Ensure main thread priority is normal */
    id = osThreadGetId();
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);

    /* Create a mail object */
    G_WaitFunc_MailQId = osMailCreate (osMailQ(MailQTout), NULL);
    ASSERT_TRUE (G_WaitFunc_MailQId != NULL);

    /* Raise priority of this thread to realtime */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityRealtime) == osOK);


    /* Measure osMailGet */


    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osMailGet (G_WaitFunc_MailQId, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osMailGet (G_WaitFunc_MailQId, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osMailGet (G_WaitFunc_MailQId, 1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsMailWait[0] && long_wait < Lim_OsMailWait[1]);
    ASSERT_TRUE (wait > Lim_OsMailWait[0] && wait < Lim_OsMailWait[1]);

    G_MailPtr = osMailAlloc (G_WaitFunc_MailQId, 0);
    ASSERT_TRUE (G_MailPtr != NULL);
    

    /* Measure osMailAlloc */

    /* Reset timer. */
    nrf_drv_timer_clear(&TIMER_TICKS);

    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osMailAlloc (G_WaitFunc_MailQId, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osMailAlloc (G_WaitFunc_MailQId, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osMailAlloc (G_WaitFunc_MailQId, 1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsMailWait[0] && long_wait < Lim_OsMailWait[1]);
    ASSERT_TRUE (wait > Lim_OsMailWait[0] && wait < Lim_OsMailWait[1]);


    /* Measure osMailCAlloc */

    /* Reset timer. */
    nrf_drv_timer_clear(&TIMER_TICKS);

    /* Measure approximate tick length */
    osDelay (1);                        /* Synchronize to the time slice */
    m0[0] = GET_TICKS;
    osMailCAlloc (G_WaitFunc_MailQId, 100);
    m0[1] = GET_TICKS;

    osDelay (1);
    m1[0] = GET_TICKS;
    osMailCAlloc (G_WaitFunc_MailQId, 101);
    m1[1] = GET_TICKS;

    long_wait = sub((sub(m1[1], m1[0])), (sub(m0[1], m0[0])));

    /* One tick should be close */
    osDelay(1);
    m0[0] = GET_TICKS;
    osMailCAlloc (G_WaitFunc_MailQId, 1);
    m0[1] = GET_TICKS;

    wait = sub(m0[1], m0[0]);

    /* Check if cycle count is within defined accuracy */
    ASSERT_TRUE (long_wait > Lim_OsMailWait[0] && long_wait < Lim_OsMailWait[1]);
    ASSERT_TRUE (wait > Lim_OsMailWait[0] && wait < Lim_OsMailWait[1]);


    /* Change priority back to normal */
    ASSERT_TRUE (osThreadSetPriority (id, osPriorityNormal) == osOK);
  }
#endif
}

/**
@}
*/ 
// end of group waitfunc_funcs

/*-----------------------------------------------------------------------------
 * End of file
 *----------------------------------------------------------------------------*/
