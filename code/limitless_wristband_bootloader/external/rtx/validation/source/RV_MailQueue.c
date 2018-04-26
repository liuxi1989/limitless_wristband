/*-----------------------------------------------------------------------------
 *      Name:         RV_MailQueue.c 
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
#define SIGNAL_MAIL_RCVD   0x01
#define SIGNAL_WAIT_TERM   0x02
#define SIGNAL_TIMER_TOUT  0x04


/* Global variable definitions */
osThreadId G_MailQ_ThreadId;
osTimerId  G_MailQ_TimerId;
int32_t    G_MailQ_TimerTimeout;
int32_t    G_MailQ_TimerPeriod;
uint32_t   G_MailQ_Counter;


#define MAIL_BUF_SZ   32
typedef struct {
  uint8_t Buf[MAIL_BUF_SZ];
} MAIL_OBJ;

#define MAILQ_SZ 8
osMailQDef (MailQ, MAILQ_SZ, MAIL_OBJ);
osMailQId MailQ_Id;


/* Timer definition */
void MailQ_TimerCallback (void const *arg);
osTimerDef (MailQ_PeriodicTimer, MailQ_TimerCallback);


/* Mailing thread definition */
void Th_MailTr (void const *arg);

osThreadDef (Th_MailTr, osPriorityNormal, 1, 0);
osThreadId  ThId_Mail;


/* Definitions for TC_MailInterrupts */
osMailQDef (MailQ_Isr, 1, uint32_t);
osMailQId   MailQId_Isr;
MAIL_OBJ   *Mo_Isr;
osEvent     MailQ_Evt_Isr;
osStatus    MailQ_Stat_Isr;


/* Definitions for TC_MailFromThreadToISR */
#define MAIL_THREAD_TO_ISR_PERIOD     2 /* Interrupt period in miliseconds    */
#define MAIL_THREAD_TO_ISR_TIMEOUT 2500 /* Timeout in ms ->  5sec @ 2ms       */

static void Isr_MailReceive (void);


/* Definitions for TC_MailFromISRToThread */
#define MAIL_ISR_TO_THREAD_PERIOD     2 /* Interrupt period in miliseconds    */
#define MAIL_ISR_TO_THREAD_TIMEOUT 2500 /* Timeout in ms ->  5sec @ 2ms       */

static void Isr_MailSend (void);

/*-----------------------------------------------------------------------------
 *      Default IRQ Handler
 *----------------------------------------------------------------------------*/
void MailQueue_IRQHandler (void) {
  
  switch (ISR_ExNum) {
    case 0:  MailQId_Isr     = osMailCreate (osMailQ (MailQ_Isr), NULL); break;
    case 1:  Mo_Isr          = (MAIL_OBJ *)osMailAlloc  (MailQ_Id,  0);  break;
    case 2:  Mo_Isr          = (MAIL_OBJ *)osMailAlloc  (MailQ_Id, 10);  break;
    case 3:  Mo_Isr          = (MAIL_OBJ *)osMailCAlloc (MailQ_Id,  0);  break;
    case 4:  Mo_Isr          = (MAIL_OBJ *)osMailCAlloc (MailQ_Id, 10);  break;
    case 5:  MailQ_Evt_Isr   = osMailGet (MailQ_Id, 0);                  break;
    case 6:  MailQ_Stat_Isr  = osMailPut (MailQ_Id, Mo_Isr);             break;
    case 7:  MailQ_Stat_Isr  = osMailFree(MailQ_Id, Mo_Isr);             break;
    
    case 8: /* TC_MailFromThreadToISR */
      Isr_MailReceive();
      break;
    case 9: /* TC_MailFromISRToThread */
      Isr_MailSend();
      break;
  }
}


/*-----------------------------------------------------------------------------
 *      Timer callback routine
 *----------------------------------------------------------------------------*/
void MailQ_TimerCallback (void const *arg) {

  if (G_MailQ_TimerTimeout > 0) {
    G_MailQ_TimerTimeout -= G_MailQ_TimerPeriod;

    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
  }
  else {
    /* Send signal if timeout expired */
    if (G_MailQ_ThreadId) {
      osSignalSet (G_MailQ_ThreadId, SIGNAL_TIMER_TOUT);
    }
  }
}


/*-----------------------------------------------------------------------------
 *      Initialization: create main mail queue
 *----------------------------------------------------------------------------*/
void CreateMailQueue (void) {
  /* Create a memory pool */
  MailQ_Id = osMailCreate (osMailQ (MailQ), NULL);
  ASSERT_TRUE (MailQ_Id != NULL);
}


/*-----------------------------------------------------------------------------
 *      Mail transceiver thread
 *----------------------------------------------------------------------------*/
void Th_MailTr (void const *arg) {
  MAIL_OBJ *mo;
  osEvent  evt;
  osStatus stat;
  uint32_t  i;
  uint32_t th_mode = *((uint32_t *)arg);
  uint8_t buf[MAIL_BUF_SZ];
  
  while (1) {
    if (th_mode == 0) {
      /* Receive mail */
      evt = osMailGet (MailQ_Id, 1000);
      ASSERT_TRUE (evt.status == osEventMail);
      
      if (evt.status == osEventMail) {
        mo = (MAIL_OBJ *)evt.value.p;
        
        /* Check mail content */
        memset(buf, 0xCC, MAIL_BUF_SZ);
        ASSERT_TRUE (memcmp(mo->Buf, buf, MAIL_BUF_SZ) == 0);
        
      }
      /* Send "mail received" signal */
      osSignalSet (G_MailQ_ThreadId, SIGNAL_MAIL_RCVD);
    }
    else if (th_mode == 1) {
      /* Allocate mail */
      mo = (MAIL_OBJ *)osMailAlloc (MailQ_Id, 0);
      ASSERT_TRUE (mo != NULL);
    
      if (mo != NULL) {
        for (i = 0; i < MAIL_BUF_SZ; i++) {
          mo->Buf[i] = 0xAA;
        }
        /* Send mail */
        stat = osMailPut (MailQ_Id, mo);
        ASSERT_TRUE (stat == osOK);
      }
    }
    
    /* Wait until terminated */
    osSignalWait (SIGNAL_WAIT_TERM, osWaitForever);
  }
}


/*-----------------------------------------------------------------------------
 * Isr_MailReceive is called from the ISR
 * This functions pools queue for mail, verifies mail and frees memory block
 *----------------------------------------------------------------------------*/
static void Isr_MailReceive (void) {
  MAIL_OBJ *mo;
  osEvent   evt;
  uint8_t buf[MAIL_BUF_SZ];

  /* Get mail from queue */
  evt = osMailGet (MailQ_Id, 0);
  if (evt.status == osEventMail) {
    mo = (MAIL_OBJ *)evt.value.p;
    ASSERT_TRUE (mo != NULL);

    if (mo != NULL) {
      /* - Verify mail content */
      memset(buf, (uint8_t)G_MailQ_Counter, MAIL_BUF_SZ);
      ASSERT_TRUE (memcmp(mo->Buf, buf, MAIL_BUF_SZ) == 0);

      G_MailQ_Counter++;                    /* Mail in sequence                     */
      
      /* - Free memory block */
      ASSERT_TRUE (osMailFree (MailQ_Id, mo) == osOK);
    }
  }
}

/*-----------------------------------------------------------------------------
 * Isr_MailSend is called from the ISR
 * This function allocates mail and puts it into mail queue
 *----------------------------------------------------------------------------*/
static void Isr_MailSend (void) {
  uint32_t  i;
  MAIL_OBJ *mo;
  osStatus  stat;

  /* Allocate mail and put it into queue */
  mo = (MAIL_OBJ *)osMailAlloc (MailQ_Id, 0);

  if (mo != NULL) {
    /* - Put data pattern into mail buffer */
    for (i = 0; i < MAIL_BUF_SZ; i++) {
      mo->Buf[i] = (uint8_t)G_MailQ_Counter;
    }

    stat = osMailPut (MailQ_Id, mo);
    if (stat == osOK) {
      /* Mail is in queue */ 
      G_MailQ_Counter++;
    }
    else {
      /* Queue is full, free allocated memory block */
      ASSERT_TRUE (stat == osErrorResource);
      ASSERT_TRUE (osMailFree (MailQ_Id, mo) == osOK);
    }
  }
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup mailqueue_funcs Mail Queue Functions
\brief Mail Queue Functions Test Cases
\details
The test cases check the osMail* functions.

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailAlloc
\details
- Allocate all mail blocks from the mail queue
- Return all mail blocks to the queue pool
*/
void TC_MailAlloc (void) {
  uint32_t  i;
  void     *mp[MAILQ_SZ];
  void     *np;
  
  if (MailQ_Id != NULL) {
    /* - Allocate all mail blocks from the mail queue */
    for (i = 0; i < MAILQ_SZ; i++) {
      mp[i] = osMailAlloc (MailQ_Id, 0);
      ASSERT_TRUE (mp[i] != NULL);
    }
    /* All mail slots allocated, next call must return NULL pointer */
    np = osMailAlloc (MailQ_Id, 0);
    ASSERT_TRUE (np == NULL);

    /* - Return all mail blocks to the queue pool */
    for (i = 0; i < MAILQ_SZ; i++) {
      if (mp[i] != NULL) {
        ASSERT_TRUE (osMailFree (MailQ_Id, mp[i]) == osOK);
      }
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailCAlloc
\details
- Allocate all mail block from the mail queue
- Init mail block with pattern
- Return all mail blocks to the queue pool
- Reallocate all mail block from the mail queue
- Check if mali block pattern was cleared to zero
- Return all mail blocks to the queue pool
*/
void TC_MailCAlloc (void) {
  uint32_t  i, j;
  void     *mp[MAILQ_SZ];
  void     *np;
  MAIL_OBJ *mo;
    
  if (MailQ_Id != NULL) {
    /* - Allocate all mail block from the mail queue */
    for (i = 0; i < MAILQ_SZ; i++) {
      mp[i] = osMailCAlloc (MailQ_Id, 0);
      ASSERT_TRUE (mp[i] != NULL);
      
      if (mp[i] != NULL) {
        mo = (MAIL_OBJ *)mp[i];
        
        /* - Init mail block with pattern */
        for (j = 0; j < MAIL_BUF_SZ; j++) {
          mo->Buf[j] = 0xAA;
        }
      }
    }
    /* All mail block allocated, next call should return NULL pointer */
    np = osMailCAlloc (MailQ_Id, 0);
    ASSERT_TRUE (np == NULL);
    
    /* - Return all mail blocks to the queue pool */
    for (i = 0; i < MAILQ_SZ; i++) {
      if (mp[i] != NULL) {
        ASSERT_TRUE (osMailFree (MailQ_Id, mp[i]) == osOK);
        mp[i] = NULL;
      }
    }
    
    /* - Reallocate all mail block from the mail queue */
    for (i = 0; i < MAILQ_SZ; i++) {
      mp[i] = osMailCAlloc (MailQ_Id, 0);
      ASSERT_TRUE (mp[i] != NULL);
      
      if (mp[i] != NULL) {
        mo = (MAIL_OBJ *)mp[i];
        
        /* - Check if mali block pattern was cleared to zero */
        for (j = 0; j < MAIL_BUF_SZ; j++) {
          ASSERT_TRUE (mo->Buf[j] == 0x00);
        }
      }
    }
    /* - Return all mail blocks to the queue pool */
    for (i = 0; i < MAILQ_SZ; i++) {
      if (mp[i] != NULL) {
        ASSERT_TRUE (osMailFree (MailQ_Id, mp[i]) == osOK);
        mp[i] = NULL;
      }
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailToThread
\details
- Create a mail receiver thread
- Allocate mail object
- Fill mail object with data
- Send mail to the receiver thread
- Wait for signal from receiver thread
- Free mail block
- Terminate receiver thread
*/
void TC_MailToThread (void) {
  uint32_t i, th_mode;
  osThreadId  id;
  osStatus    stat;
  osEvent     evt;
  MAIL_OBJ   *mo;

  G_MailQ_ThreadId = osThreadGetId();
  ASSERT_TRUE (G_MailQ_ThreadId != NULL);

  /* - Create a mail receiver thread */
  th_mode = 0;
  id = osThreadCreate (osThread (Th_MailTr), &th_mode);
  ASSERT_TRUE (id != NULL);

  if (G_MailQ_ThreadId && id) {
    /* - Allocate mail object */
    mo = (MAIL_OBJ *)osMailAlloc (MailQ_Id, 100);
    ASSERT_TRUE (mo != NULL);

    if (mo != NULL) {
      /* - Fill mail object with data */
      for (i = 0; i < MAIL_BUF_SZ; i++) {
        mo->Buf[i] = 0xCC;
      }
      /* - Send mail to the receiver thread */
      stat = osMailPut (MailQ_Id, mo);
      ASSERT_TRUE (stat == osOK);

      /* - Wait for signal from receiver thread */
      evt = osSignalWait (SIGNAL_MAIL_RCVD, 20);
      ASSERT_TRUE (evt.status == osEventSignal);
      ASSERT_TRUE (evt.value.signals == SIGNAL_MAIL_RCVD);

      /* - Free mail block */
      ASSERT_TRUE (osMailFree (MailQ_Id, mo) == osOK);
    }
    /* - Terminate receiver thread */
    ASSERT_TRUE (osThreadTerminate (id) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailFromThread
\details
- Create a mail sender thread
- Wait for mail from sender thread
- Verify mail content
- Free mail block
- Terminate sender thread
*/
void TC_MailFromThread (void) {
  uint32_t    th_mode;
  osThreadId  id;
  osEvent     evt;
  MAIL_OBJ   *mo;
  uint8_t buf[MAIL_BUF_SZ];
  
  /* - Create a mail sender thread */
  th_mode = 1;
  id = osThreadCreate (osThread (Th_MailTr), &th_mode);
  ASSERT_TRUE (id != NULL);
  
  if (id != NULL) {
    /* - Wait for mail from sender thread */
    evt = osMailGet (MailQ_Id, 1000);
    ASSERT_TRUE (evt.status == osEventMail);
    
    if (evt.status == osEventMail) {
      mo = (MAIL_OBJ *)evt.value.p;
      
      /* - Verify mail content */
      memset(buf, 0xAA, MAIL_BUF_SZ);
      ASSERT_TRUE (memcmp(mo->Buf, buf, MAIL_BUF_SZ) == 0);
      
      /* - Free mail block */
      ASSERT_TRUE (osMailFree (MailQ_Id, mo) == osOK);
    }
    /* - Terminate sender thread */
    ASSERT_TRUE (osThreadTerminate (id) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailTimeout
\details
- Wait for mail until time-out expires
- osMailGet must return status code osEventTimeout
*/
void TC_MailTimeout (void) {
  ASSERT_TRUE (osMailGet (MailQ_Id, 100).status == osEventTimeout);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailParam
\details
- Test mail queue management functions with invalid parameters
*/
void TC_MailParam (void) {
  uint32_t val = 0;
  
  ASSERT_TRUE (osMailCreate (NULL, NULL) == NULL);
  ASSERT_TRUE (osMailAlloc  (NULL, 0) == NULL);
  ASSERT_TRUE (osMailCAlloc (NULL, 0) == NULL);
  
  ASSERT_TRUE (osMailFree (NULL, NULL)     == osErrorParameter);
  ASSERT_TRUE (osMailFree (MailQ_Id, NULL) == osErrorValue);
  ASSERT_TRUE (osMailFree (MailQ_Id, &val) == osErrorValue);
  
  ASSERT_TRUE (osMailGet (NULL, 0).status  == osErrorParameter);
  
  ASSERT_TRUE (osMailPut (NULL, NULL)      == osErrorParameter);
  ASSERT_TRUE (osMailPut (MailQ_Id, NULL)  == osErrorValue);
  ASSERT_TRUE (osMailPut (MailQ_Id, &val)  == osErrorValue);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailInterrupts
\details
- Call osMailAlloc without time-out from the ISR
- Call osMailAlloc with time-out from the ISR
- Call osMailCAlloc without time-out from the ISR
- Call osMailCAlloc with time-out from the ISR
- Call osMailGet from the ISR
- Call osMailPut from the ISR
- Call osMailFree from the ISR
*/
void TC_MailInterrupts (void) {
  uint32_t i;
  MAIL_OBJ *mo;
  osEvent   evt;
  uint8_t buf[MAIL_BUF_SZ];
  
  TST_IRQHandler = MailQueue_IRQHandler;
  
  NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);
  
  ISR_ExNum  = 0; /* Test: osMailCreate */
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (MailQId_Isr == NULL);
  
  ISR_ExNum  = 1; /* Test: osMailAlloc, no time-out */  
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (Mo_Isr != NULL);
  
  if (Mo_Isr != NULL) {
    ASSERT_TRUE (osMailFree (MailQ_Id, Mo_Isr) == osOK);
  }

  ISR_ExNum  = 2; /* Test: osMailAlloc, with time-out */
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (Mo_Isr == NULL);
  
  if (Mo_Isr != NULL) {
    ASSERT_TRUE (osMailFree (MailQ_Id, Mo_Isr) == osOK);
  }

  ISR_ExNum  = 3; /* Test: osMailCAlloc, no time-out */
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (Mo_Isr != NULL);
  
  if (Mo_Isr != NULL) {
    ASSERT_TRUE (osMailFree (MailQ_Id, Mo_Isr) == osOK);
  }

  ISR_ExNum  = 4; /* Test: osMailCAlloc, with time-out */
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (Mo_Isr == NULL);
  
  if (Mo_Isr != NULL) {
    ASSERT_TRUE (osMailFree (MailQ_Id, Mo_Isr) == osOK);
  }

  ISR_ExNum  = 5; /* Test: osMailGet  */

  /* Prepare mail */
  mo = (MAIL_OBJ *)osMailAlloc (MailQ_Id, 0);
  ASSERT_TRUE (mo != NULL);

  if (mo != NULL) {
    for (i = 0; i < MAIL_BUF_SZ; i++) {
      mo->Buf[i] = 0xCC;
    }
    ASSERT_TRUE (osMailPut (MailQ_Id, mo) == osOK);
  }
  NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
  __DMB();
  ASSERT_TRUE (MailQ_Evt_Isr.status == osEventMail);

  if (MailQ_Evt_Isr.status == osEventMail) {
    mo = (MAIL_OBJ *)MailQ_Evt_Isr.value.p;

    memset(buf, 0xCC, MAIL_BUF_SZ);
    ASSERT_TRUE (memcmp(mo->Buf, buf, MAIL_BUF_SZ) == 0);

    ASSERT_TRUE (osMailFree (MailQ_Id, mo) == osOK);
  }

  ISR_ExNum = 6; /* Test: osMailPut  */

  /* Allocate mail */
  Mo_Isr = (MAIL_OBJ *)osMailAlloc (MailQ_Id, 0);
  ASSERT_TRUE (Mo_Isr != NULL);

  if (Mo_Isr != NULL) {
    /* Set pattern */
    for (i = 0; i < MAIL_BUF_SZ; i++) {
      Mo_Isr->Buf[i] = 0xAA;
    }
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();

    evt = osMailGet (MailQ_Id, 1000);
    ASSERT_TRUE (evt.status == osEventMail);
    
    if (evt.status == osEventMail) {
      mo = (MAIL_OBJ *) evt.value.p;
      /* Check pattern */
      memset(buf, 0xAA, MAIL_BUF_SZ);
      ASSERT_TRUE (memcmp(mo->Buf, buf, MAIL_BUF_SZ) == 0);
      
      /* Release mail */
      ASSERT_TRUE (osMailFree (MailQ_Id, mo) == osOK);
    }
    else {
      /* Release mail if ISR test failed */
      ASSERT_TRUE (osMailFree (MailQ_Id, Mo_Isr) == osOK);
    }
  }

  ISR_ExNum = 7; /* Test: osMailFree */

  /* Allocate mail */
  Mo_Isr = (MAIL_OBJ *)osMailAlloc (MailQ_Id, 0);
  ASSERT_TRUE (Mo_Isr != NULL);

  if (Mo_Isr != NULL) {
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (MailQ_Stat_Isr == osOK);
    
    if (MailQ_Stat_Isr != osOK) {
      /* Release mail if this was not done in the ISR */
      ASSERT_TRUE (osMailFree (MailQ_Id, Mo_Isr) == osOK);
    }
  }

  NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailFromThreadToISR
\details
- Continuously allocate mail, fill it with pattern and put it into queue
- Periodically trigger ISR and get, verify, free mail from it
*/
void TC_MailFromThreadToISR (void) {
  MAIL_OBJ *mo;
  osStatus  stat;
  osEvent   evt;
  uint8_t   cnt;
  uint32_t  i;
  
  TST_IRQHandler = MailQueue_IRQHandler;
  
  ISR_ExNum = 8;                      /* Set case number in the ISR         */ 
  NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);

  G_MailQ_ThreadId = osThreadGetId ();
  ASSERT_TRUE (G_MailQ_ThreadId != NULL);

  if (G_MailQ_ThreadId != NULL) {
    /* Ensure mail queue is empty */
    do {
      evt = osMailGet (MailQ_Id, 0);
      if (evt.status == osEventMail) {
        ASSERT_TRUE (osMailFree (MailQ_Id, evt.value.p) == osOK);
      }
    }
    while (evt.status != osOK);
    
    /* Create and start a periodic timer with defined period */
    G_MailQ_TimerPeriod  = MAIL_THREAD_TO_ISR_PERIOD;
    G_MailQ_TimerTimeout = MAIL_THREAD_TO_ISR_TIMEOUT;

    G_MailQ_TimerId = osTimerCreate (osTimer (MailQ_PeriodicTimer), osTimerPeriodic, NULL);
    ASSERT_TRUE (G_MailQ_TimerId != NULL);

    if (G_MailQ_TimerId != NULL) {
      stat = osTimerStart (G_MailQ_TimerId, MAIL_THREAD_TO_ISR_PERIOD);
      ASSERT_TRUE (stat == osOK);

      /* Begin sending mail */
      if (stat == osOK) {
        /* - Allocate, prepare mail and put it into mail queue */
        G_MailQ_Counter = 0;
        cnt = 0;
        do {
          mo = (MAIL_OBJ *)osMailAlloc (MailQ_Id, 0);
          if (mo != NULL) {
            for (i = 0; i < MAIL_BUF_SZ; i++) {
              mo->Buf[i] = cnt;
            }
            stat = osMailPut (MailQ_Id, mo);

            if  (stat == osOK) {
              cnt++;
            }
          }
        }
        while (osSignalWait (SIGNAL_TIMER_TOUT, 0).status != osEventSignal);
      }
      
      /* Delete timer object */
      ASSERT_TRUE (osTimerDelete (G_MailQ_TimerId) == osOK);
    }
  }
  NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_MailFromISRToThread
\details
- Periodically allocate and put mail into queue from the ISR
- Pool mail queue from a thread
- When mail available get if from queue and verify its content
- Free mail block
*/
void TC_MailFromISRToThread (void) {
  MAIL_OBJ *mo;
  osStatus  stat;
  osEvent   evt;
  uint8_t   cnt;
  uint8_t buf[MAIL_BUF_SZ];

  TST_IRQHandler = MailQueue_IRQHandler;
   
  ISR_ExNum = 9;                        /* Set case number in the ISR         */
  NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);
  
  G_MailQ_ThreadId = osThreadGetId ();
  ASSERT_TRUE (G_MailQ_ThreadId != NULL);

  if (G_MailQ_ThreadId != NULL) {
    /* Ensure mail queue is empty */
    do {
      evt = osMailGet (MailQ_Id, 0);
      if (evt.status == osEventMail) {
        ASSERT_TRUE (osMailFree (MailQ_Id, evt.value.p) == osOK);
      }
    }
    while (evt.status != osOK);

    /* Create and start a periodic timer with defined period */
    G_MailQ_TimerPeriod  = MAIL_ISR_TO_THREAD_PERIOD;
    G_MailQ_TimerTimeout = MAIL_ISR_TO_THREAD_TIMEOUT;

    G_MailQ_TimerId = osTimerCreate (osTimer (MailQ_PeriodicTimer), osTimerPeriodic, NULL);
    ASSERT_TRUE (G_MailQ_TimerId != NULL);

    if (G_MailQ_TimerId != NULL) {
      G_MailQ_Counter = 0;
      cnt = 0;

      stat = osTimerStart (G_MailQ_TimerId, MAIL_ISR_TO_THREAD_PERIOD);
      ASSERT_TRUE (stat == osOK);

      if (stat == osOK) {
        /* Begin receiving mail */
        do {
          evt = osMailGet (MailQ_Id, 0);
          if (evt.status == osEventMail) {
            mo = (MAIL_OBJ *)evt.value.p;
            ASSERT_TRUE (mo != NULL);

            if (mo != NULL) {
              /* - Verify mail content */
              memset(buf, cnt, MAIL_BUF_SZ);
              ASSERT_TRUE (memcmp(mo->Buf, buf, MAIL_BUF_SZ) == 0);

              cnt++;

              /* - Free memory block */
              ASSERT_TRUE (osMailFree (MailQ_Id, mo) == osOK);
            }
          }
        }
        while (osSignalWait (SIGNAL_TIMER_TOUT, 0).status != osEventSignal);
      }

      /* Delete timer object */
      ASSERT_TRUE (osTimerDelete (G_MailQ_TimerId) == osOK);
    }
  }
  NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
}

/**
@}
*/ 
// end of group mailqueue_funcs

/*-----------------------------------------------------------------------------
 * End of file
 *----------------------------------------------------------------------------*/
