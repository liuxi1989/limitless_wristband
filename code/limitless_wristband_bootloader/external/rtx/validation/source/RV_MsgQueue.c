/*-----------------------------------------------------------------------------
 *      Name:         RV_MsgQueue.c 
 *      Purpose:      CMSIS RTOS validation tests implementation
 *-----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include <string.h>
#include "RV_Typedefs.h"
#include "cmsis_rv.h"
#include "cmsis_os.h"

/*-----------------------------------------------------------------------------
 *      Test implementation
 *----------------------------------------------------------------------------*/
/* Signal definitions */
#define SIGNAL_TIMER_TOUT  0x04

/* Global variable definitions */
osThreadId G_MsgQ_ThreadId;
osTimerId  G_MsgQ_TimerId;
int32_t    G_MsgQ_TimerTimeout;
int32_t    G_MsgQ_TimerPeriod;
uint32_t   G_MsgQ_Counter;


#define MSGQ_SZ 16
osMessageQDef (MsgQ, MSGQ_SZ, uint32_t);
osMessageQId MsgQ_Id;


/* Timer definition */
void MsgQ_TimerCallback (void const *arg);
osTimerDef (MsgQ_PeriodicTimer, MsgQ_TimerCallback);


/* Definitions for TC_MsgQWait */
void Th_MsgWait (void const *arg);
osThreadDef (Th_MsgWait, osPriorityAboveNormal, 1, 0);

uint8_t MsgWaitCnt;


/* Definitions for TC_MsgQInterrupts */
osMessageQDef (MsgQ_Isr, MSGQ_SZ, uint32_t);
osMessageQId MsgQId_Isr;
osStatus     MsgQSt_Isr;
osEvent      MsgQEv_Isr;

/* Definitions for TC_MsgFromThreadToISR */
#define MSG_THREAD_TO_ISR_PERIOD     2  /* Interrupt period in miliseconds    */
#define MSG_THREAD_TO_ISR_TIMEOUT 2500  /* Timeout in ms ->  5sec @ 2ms       */

static void Isr_MsgReceive (void);


/* Definitions for TC_MsgFromISRToThread */
#define MSG_ISR_TO_THREAD_PERIOD     2  /* Interrupt period in miliseconds    */
#define MSG_ISR_TO_THREAD_TIMEOUT 2500  /* Timeout in ms ->  5sec @ 2ms       */

static void Isr_MsgSend (void);


/*-----------------------------------------------------------------------------
 *      Default IRQ Handler
 *----------------------------------------------------------------------------*/
void MsgQueue_IRQHandler (void) {
  
  switch (ISR_ExNum) {
    case 0: MsgQId_Isr = osMessageCreate (osMessageQ (MsgQ_Isr), NULL); break;
    case 1: MsgQSt_Isr = osMessagePut (MsgQId_Isr, 0x01,  0);           break;
    case 2: MsgQSt_Isr = osMessagePut (MsgQId_Isr, 0x02, 10);           break;
    case 3: MsgQEv_Isr = osMessageGet (MsgQId_Isr,  0);                 break;
    case 4: MsgQEv_Isr = osMessageGet (MsgQId_Isr, 10);                 break;
    
    case 5: /* TC_MsgFromThreadToISR */
      Isr_MsgReceive ();
      break;
    
    case 6: /* TC_MsgFromISRToThread */
      Isr_MsgSend ();
      break;
  }
}


/*-----------------------------------------------------------------------------
 *      Timer callback routine
 *----------------------------------------------------------------------------*/
void MsgQ_TimerCallback (void const *arg) {

  if (G_MsgQ_TimerTimeout > 0) {
    G_MsgQ_TimerTimeout -= G_MsgQ_TimerPeriod;

    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
  }
  else {
    /* Send signal if timeout expired */
    if (G_MsgQ_ThreadId) {
      osSignalSet (G_MsgQ_ThreadId, SIGNAL_TIMER_TOUT);
    }
  }
}


/*-----------------------------------------------------------------------------
 *      Initialization: create main message queue
 *----------------------------------------------------------------------------*/
void CreateMessageQueue (void) {
  /* Create a memory pool */
  MsgQ_Id = osMessageCreate (osMessageQ (MsgQ), NULL);
  ASSERT_TRUE (MsgQ_Id != NULL);
}

/*-----------------------------------------------------------------------------
 *      Message waiting thread
 *----------------------------------------------------------------------------*/
void Th_MsgWait (void const *arg) {
  uint32_t *p = (uint32_t *)arg;
  osEvent evt;
  
  while (1) {
    /* Increment a counter */
    MsgWaitCnt += 1;
    /* Wait until message arrives */
    evt = osMessageGet (MsgQ_Id, osWaitForever);
    ASSERT_TRUE (evt.status == osEventMessage);
    /* Increment a counter */
    MsgWaitCnt += 1;
    
    if (p && evt.status == osEventMessage) {
      /* Return counter value */
      *p = (uint32_t)evt.value.p;
      return;
    }
  }
}

/*-----------------------------------------------------------------------------
 * Isr_MsgReceive is called from the ISR
 * This functions pools queue for message and verifies it
 *----------------------------------------------------------------------------*/
static void Isr_MsgReceive (void) {
  osEvent evt;
  
  /* Get messages from queue */
  evt = osMessageGet (MsgQ_Id, 0);
  
  if (evt.status == osEventMessage) {
    ASSERT_TRUE (evt.value.v == G_MsgQ_Counter);
    G_MsgQ_Counter++;
  }
  else {
    /* MessageGet should only fail if there is no message in queue */
    ASSERT_TRUE (evt.status == osOK);
  }
}

/*-----------------------------------------------------------------------------
 * Isr_MsgSend is called from the ISR
 * This functions puts a message into the message queue
 *----------------------------------------------------------------------------*/
static void Isr_MsgSend (void) {
  osStatus stat;
  
  stat = osMessagePut (MsgQ_Id, G_MsgQ_Counter, 0);
  if (stat == osOK) {
    /* Message is in queue */
    G_MsgQ_Counter++;
  }
  else {
    /* Message put should only fail if queue full */
    ASSERT_TRUE (stat == osErrorResource);
  }
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup msgqueue_funcs Message Queue Functions
\brief Message Queue Functions Test Cases
\details
The test cases check the osMessage* functions.

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MsgQBasic
\details
- Fill message queue with messages
- Get all messages from queue
- Check if messages are correct
*/
void TC_MsgQBasic (void) {
  osEvent  evt;
  uint32_t i, txi, rxi;
  
  if (MsgQ_Id != NULL) {
    /* - Fill message queue with messages */
    for (txi = 0, i = 0; i < MSGQ_SZ; i++, txi++) {
      ASSERT_TRUE (osMessagePut (MsgQ_Id, txi, 0) == osOK);
    }
    /* Message queue full, check resource error */
    ASSERT_TRUE (osMessagePut (MsgQ_Id, txi,  0) == osErrorResource);
    /* Message queue full, check timeout error */
    ASSERT_TRUE (osMessagePut (MsgQ_Id, txi, 10) == osErrorTimeoutResource);
    
    /* - Get all messages from queue */
    for (txi = 0, i = 0; i < MSGQ_SZ; i++, txi++) {
      evt = osMessageGet (MsgQ_Id, 0);
      ASSERT_TRUE (evt.status == osEventMessage);
      /* - Check if messages are correct */
      if (evt.status == osEventMessage) {
        rxi = evt.value.v;
        ASSERT_TRUE (rxi == txi);
      }
    }
    /* Message queue empty, check return values */
    ASSERT_TRUE (osMessageGet (MsgQ_Id,  0).status == osOK);
    ASSERT_TRUE (osMessageGet (MsgQ_Id, 10).status == osEventTimeout);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MsgQWait
\details
- Reset global counter
- Create a thread that increments a counter and waits for message
- Verify if counter incremented
- Send message to the waiting thread
- Verify if counter incremented
- Verify if message received
*/
void TC_MsgQWait (void) {
  uint32_t cnt = 0;
  
  if (MsgQ_Id != NULL) {
    /* - Reset global counter */
    MsgWaitCnt = 0;
    /* - Create a thread that increments a counter and waits for message */
    ASSERT_TRUE (osThreadCreate (osThread (Th_MsgWait), &cnt) != NULL);
    /* - Verify if counter incremented */
    ASSERT_TRUE (MsgWaitCnt == 1);
    /* - Send message to the waiting thread */
    ASSERT_TRUE (osMessagePut (MsgQ_Id, 2, osWaitForever) == osOK);
    /* - Verify if counter incremented */
    ASSERT_TRUE (MsgWaitCnt == 2);
    /* - Verify if message received */
    ASSERT_TRUE (MsgWaitCnt == cnt);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MsgQParam
\details
- Test message queue management functions with invalid parameters
*/
void TC_MsgQParam (void) {
  uint32_t info = 0;
  
  ASSERT_TRUE (osMessageCreate (NULL, NULL)  == NULL);
  
  ASSERT_TRUE (osMessageGet (NULL, 0).status == osErrorParameter);
  
  ASSERT_TRUE (osMessagePut (NULL, info, 0)  == osErrorParameter);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MsgQInterrupts
\details
- Call all message queue management functions from the ISR
*/
void TC_MsgQInterrupts (void) {
  
  TST_IRQHandler = MsgQueue_IRQHandler;
  
  NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);
  
  ISR_ExNum = 0;  /* Test: osMessageCreate */
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (MsgQId_Isr == NULL);
  
  if (MsgQId_Isr == NULL) {
    MsgQId_Isr = osMessageCreate (osMessageQ (MsgQ_Isr), NULL);
    ASSERT_TRUE (MsgQId_Isr != NULL);
    
    if (MsgQId_Isr != NULL) {
      ISR_ExNum = 1; /* Test: osMessagePut, no time-out */
      NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
      __DMB();
      ASSERT_TRUE (MsgQSt_Isr == osOK);
      
      ISR_ExNum = 2; /* Test: osMessagePut, with time-out */
      NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
      __DMB();
      ASSERT_TRUE (MsgQSt_Isr == osErrorParameter);
      
      ISR_ExNum = 3; /* Test: osMessageGet, no time-out */
      NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
      __DMB();
      ASSERT_TRUE (MsgQEv_Isr.status == osEventMessage);

      ISR_ExNum = 4; /* Test: osMessageGet, with time-out */
      NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
      __DMB();
      ASSERT_TRUE (MsgQEv_Isr.status == osErrorParameter);
    }
  }
  
  NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MsgFromThreadToISR
\details
- Continuously put messages into queue from a thread
- Periodically trigger ISR, then get and verify message in it
*/
void TC_MsgFromThreadToISR (void) {
  osStatus stat;
  osEvent  evt;
  uint32_t cnt;
  
  TST_IRQHandler = MsgQueue_IRQHandler;
  
  ISR_ExNum = 5;                        /* Set case number in the ISR         */
  NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);

  G_MsgQ_ThreadId = osThreadGetId ();
  ASSERT_TRUE (G_MsgQ_ThreadId != NULL);

  if (G_MsgQ_ThreadId != NULL) {
    /* Ensure message queue is empty */
    do {
      evt = osMessageGet (MsgQ_Id, 0);
    }
    while (evt.status != osOK);
    
    /* Create and start a periodic timer with defined period */
    G_MsgQ_TimerPeriod  = MSG_THREAD_TO_ISR_PERIOD;
    G_MsgQ_TimerTimeout = MSG_THREAD_TO_ISR_TIMEOUT;
    
    G_MsgQ_TimerId = osTimerCreate (osTimer (MsgQ_PeriodicTimer), osTimerPeriodic, NULL);
    ASSERT_TRUE (G_MsgQ_TimerId != NULL);
    
    if (G_MsgQ_TimerId != NULL) {
      stat = osTimerStart (G_MsgQ_TimerId, MSG_THREAD_TO_ISR_PERIOD);
      ASSERT_TRUE (stat == osOK);
      
      if (stat == osOK) {
        /* Start putting messages into queue */
        G_MsgQ_Counter = 0;
        cnt = 0;
        do {
          stat = osMessagePut (MsgQ_Id, cnt, 0);
          if (stat == osOK) {
            /* Message is in queue */
            cnt++;
          }
          else {
            /* MesssagePut should only fail if queue full */
            ASSERT_TRUE (stat == osErrorResource);
          }
        }
        while (osSignalWait (SIGNAL_TIMER_TOUT, 0).status != osEventSignal);
      }

      /* Delete timer object */
      ASSERT_TRUE (osTimerDelete (G_MsgQ_TimerId) == osOK);
    }
  }
  NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MsgFromISRToThread
\details
- Periodically put messages into queue from the ISR
- Pool for messages in a thread
- Get and verify these messages
*/
void TC_MsgFromISRToThread (void) {
  osStatus stat;
  osEvent  evt;
  uint32_t cnt;
  
  TST_IRQHandler = MsgQueue_IRQHandler;
  
  ISR_ExNum = 6;                        /* Set case number in the ISR         */
  NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);
  
  G_MsgQ_ThreadId = osThreadGetId ();
  ASSERT_TRUE (G_MsgQ_ThreadId != NULL);

  if (G_MsgQ_ThreadId != NULL) {
    /* Ensure message queue is empty */
    do {
      evt = osMessageGet (MsgQ_Id, 0);
    }
    while (evt.status != osOK);

    /* Create and start a periodic timer with defined period */
    G_MsgQ_TimerPeriod  = MSG_ISR_TO_THREAD_PERIOD;
    G_MsgQ_TimerTimeout = MSG_ISR_TO_THREAD_TIMEOUT;

    G_MsgQ_TimerId = osTimerCreate (osTimer (MsgQ_PeriodicTimer), osTimerPeriodic, NULL);
    ASSERT_TRUE (G_MsgQ_TimerId != NULL);

    if (G_MsgQ_TimerId != NULL) {
      G_MsgQ_Counter = 0;
      cnt = 0;

      stat = osTimerStart (G_MsgQ_TimerId, MSG_ISR_TO_THREAD_PERIOD);
      ASSERT_TRUE (stat == osOK);

      if (stat == osOK) {
        /* Begin receiving messages */
        do {
          evt = osMessageGet (MsgQ_Id, 0);
          
          if (evt.status == osEventMessage) {
            ASSERT_TRUE (evt.value.v == cnt);
            cnt++;
          }
          else {
            /* Message get should only fail if queue empty */
            ASSERT_TRUE (evt.status == osOK);
          }
        }
        while (osSignalWait (SIGNAL_TIMER_TOUT, 0).status != osEventSignal);
      }

      /* Delete timer object */
      ASSERT_TRUE (osTimerDelete (G_MsgQ_TimerId) == osOK);
    }
  }
  NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
}

/**
@}
*/ 
// end of group msgqueue_funcs

/*-----------------------------------------------------------------------------
 * End of file
 *----------------------------------------------------------------------------*/
