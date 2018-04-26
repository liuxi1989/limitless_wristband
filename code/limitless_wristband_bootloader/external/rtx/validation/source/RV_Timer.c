/*-----------------------------------------------------------------------------
 *      Name:         RV_Timer.c 
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

/* Timer callback prototypes */
void TimCb_Oneshot  (void const *arg);
void TimCb_Periodic (void const *arg);
void TimCb_Running  (void const *arg);
void TimCb_Dummy    (void const *arg);

volatile uint32_t Tim_Var, Tim_Var_Os, Tim_Var_Per;

osTimerDef (Tim_OneShot,  TimCb_Oneshot);
osTimerDef (Tim_Periodic, TimCb_Periodic);
osTimerDef (Tim_Running,  TimCb_Running);
osTimerDef (Tim_Dummy,    TimCb_Dummy);

osTimerId TimId_Running;                /* Timer ID for timer running in ISR  */
osTimerId TimId_Isr;                    /* Timer ID returned from the ISR     */
osStatus  TimSt_Isr;                    /* Status returned from the ISR       */


/*-----------------------------------------------------------------------------
 *      One shoot timer
 *----------------------------------------------------------------------------*/
void TimCb_Oneshot  (void const *arg) {
  uint32_t var = *(uint32_t *)arg;
  
  ASSERT_TRUE (var != 0);
  Tim_Var_Os += 1;
}


/*-----------------------------------------------------------------------------
 *      Periodic timer
 *----------------------------------------------------------------------------*/
void TimCb_Periodic (void const *arg) {
  uint32_t var = *(uint32_t *)arg;
  
  ASSERT_TRUE (var != 0);
  Tim_Var_Per += 1;
}


/*-----------------------------------------------------------------------------
 *      Running (periodic) timer
 *----------------------------------------------------------------------------*/
void TimCb_Running (void const *arg) {
  Tim_Var += 1;
}

/*-----------------------------------------------------------------------------
 *      Dummy timer callback
 *----------------------------------------------------------------------------*/
void TimCb_Dummy (void const *arg) {
}

/*-----------------------------------------------------------------------------
 *      Default IRQ Handler
 *----------------------------------------------------------------------------*/
void Timer_IRQHandler (void) {
  switch (ISR_ExNum) {
    case 0: TimId_Isr = osTimerCreate (osTimer (Tim_Dummy), osTimerOnce,     NULL); break;
    case 1: TimId_Isr = osTimerCreate (osTimer (Tim_Dummy), osTimerPeriodic, NULL); break;
    case 2: TimSt_Isr = osTimerStart  ((TimId_Running), 100); break;
    case 3: TimSt_Isr = osTimerStop    (TimId_Running);       break;
    case 4: TimSt_Isr = osTimerDelete  (TimId_Running);       break;
  }
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup timer_func Timer Functions
\brief Timer Functions Test Cases
\details
The test cases check the osTimer* functions.

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_TimerOneShot
\details
- Create a one shoot timer
- Ensure timer is not running
- Start timer and check that callback was called only once
- Ensure correct status reporting
- Delete the timer
*/
void TC_TimerOneShot (void) {
  uint32_t  i, var = 1;
  osTimerId id;

  /* - Create a one shoot timer */
  id = osTimerCreate (osTimer (Tim_OneShot), osTimerOnce, &var);
  ASSERT_TRUE (id != NULL);

  if (id) {
    /* - Ensure timer is not running */
    ASSERT_TRUE (osTimerStop (id) == osErrorResource);
    
    /* - Start timer and check that callback was called only once */
    ASSERT_TRUE (osTimerStart (id, 10) == osOK);
    
    /* Wait for timer interval */
    for (i = 1000000; i; i--) {
      if (Tim_Var_Os > 0) {
        break;
      }
    }
    ASSERT_TRUE (i != 0);
    ASSERT_TRUE (Tim_Var_Os > 0);
    
    /* Wait another interval */
    for (i = 1000000; i; i--) {
      if (Tim_Var_Os > 1) {
        break;
      }
    }
    ASSERT_TRUE (i == 0);
    ASSERT_TRUE (Tim_Var_Os == 1);

    /* - Ensure correct status reporting */
    ASSERT_TRUE (osTimerStop (id) == osErrorResource);
    /* - Delete the timer */
    ASSERT_TRUE (osTimerDelete (id) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_TimerPeriodic
\details
- Create a periodic timer
- Ensure timer is not running
- Start timer and check if callback function is called more than once
- Stop the timer
- Ensure correct status reporting
- Delete the timer
*/
void TC_TimerPeriodic (void) {
  uint32_t i, var = 1;
  osTimerId id;

  /* - Create a periodic timer */
  id = osTimerCreate (osTimer (Tim_Periodic), osTimerPeriodic, &var);
  ASSERT_TRUE (id != NULL);

  if (id) {
    /* - Ensure timer is not running */
    ASSERT_TRUE (osTimerStop (id) == osErrorResource);
    /* - Start timer and check if callback function is called more than once */
    ASSERT_TRUE (osTimerStart (id, 10) == osOK);
    
    /* Wait for timer interval */
    for (i = 10000000; i; i--) {
      if (Tim_Var_Per > 2) {
        break;
      }
    }
    ASSERT_TRUE (i != 0);
    ASSERT_TRUE (Tim_Var_Per > 2);
    
    /* - Stop the timer */
    ASSERT_TRUE (osTimerStop (id) == osOK);
    /* - Ensure correct status reporting */
    ASSERT_TRUE (osTimerStop (id) == osErrorResource);
    /* - Delete the timer */
    ASSERT_TRUE (osTimerDelete (id) == osOK);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_TimerParam
\details
- Test timer management functions with invalid parameters
*/
void TC_TimerParam (void) {
#if (PARAM_CHECKING)
  osTimerId id;
#endif
  
  /* - Test invalid timer definition on osTimercreate */
  ASSERT_TRUE (osTimerCreate (NULL, osTimerOnce, NULL) == NULL);
  
  /* - Test invalid timer id on osTimerStart */
  ASSERT_TRUE (osTimerStart (NULL, 0) == osErrorParameter);
  
  /* - Test invalid timer id on osTimerStop */
  ASSERT_TRUE (osTimerStop (NULL)   == osErrorParameter);
  
  /* - Test invalid timer id on osTimerDelete */
  ASSERT_TRUE (osTimerDelete (NULL) == osErrorParameter);

#if (PARAM_CHECKING)
  /* - Test uninitialized parameter on osTimerStart */
  ASSERT_TRUE (osTimerStart (id, 0) == osErrorParameter);
  
  /* - Test uninitialized parameter on osTimerStop */
  ASSERT_TRUE (osTimerStop  (id)    == osErrorParameter);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: TC_TimerInterrupts
\details
- Call all timer management functions from the ISR
*/
void TC_TimerInterrupts (void) {
  
  TST_IRQHandler = Timer_IRQHandler;
  
  TimId_Running = osTimerCreate (osTimer (Tim_Running), osTimerPeriodic, NULL);
  ASSERT_TRUE (TimId_Running != NULL);
  
  if (TimId_Running != NULL) {
    NVIC_EnableIRQ((IRQn_Type)SWI_HANDLER);
    
    ISR_ExNum = 0; /* Test: osTimerCreate (One Shoot) */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (TimId_Isr == NULL);
 
    ISR_ExNum = 1; /* Test: osTimerCreate (Periodic) */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (TimId_Isr == NULL);
    
    ISR_ExNum = 2; /* Test: osTimerStart */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (TimSt_Isr == osErrorISR);

    ISR_ExNum = 3; /* Test: osTimerStop */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (TimSt_Isr == osErrorISR);
    
    ISR_ExNum = 4; /* Test: osTimerDelete */
    NVIC_SetPendingIRQ((IRQn_Type)SWI_HANDLER);
    __DMB();
    ASSERT_TRUE (TimSt_Isr == osErrorISR);
    
    NVIC_DisableIRQ((IRQn_Type)SWI_HANDLER);
  }
}

/**
@}
*/ 
// end of group timer_func

/*-----------------------------------------------------------------------------
 * End of file
 *----------------------------------------------------------------------------*/
